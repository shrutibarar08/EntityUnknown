Texture2D Src : register(t0);
SamplerState Samp : register(s0);

cbuffer POSTFX_COMMON_PS_CB : register(b0)
{
    float iTime;
    float iTimerDelta;
    float iTimeFrameRate;
    int iFrame;

    float3 iResolution;
    float _padRes;

    float4 iMouse;

    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;

    float3 CameraPosition;
    float _padCam;

    // ---------------- Toon parameter mapping ----------------
    // ExtraPram_1: x = levels (>=2, default 4)
    //              y = contrast (0..3, default 1.0)
    //              z = saturation (0..3, default 1.0)
    //              w = gamma (0.2..3, default 1.0)
    //
    // ExtraPram_2: x = edgeThickness (pixel radius, ~0.5..3, default 1.0)
    //              y = edgeStrength (0..4, default 1.5)
    //              z = edgeThreshold (0..1, default 0.25)
    //              w = edgeSoftness (0..1, default 0.10)
    //
    // ExtraPram_3: x,y,z = edgeColor (default 0,0,0)
    //              w     = globalBlend with source (0..1, default 1 for full toon)
    float4 ExtraPram_1;
    float4 ExtraPram_2;
    float4 ExtraPram_3;
};

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// ------------------- helpers -------------------
float3 ApplyContrast(float3 c, float k)
{
    return lerp(0.5.xxx, c, k) + (c - 0.5.xxx) * (k - 1.0);
}
float3 ApplySaturation(float3 c, float s)
{
    float l = dot(c, float3(0.299, 0.587, 0.114));
    return lerp(l.xxx, c, s);
}
float3 ApplyGamma(float3 c, float g)
{
    return pow(saturate(c), 1.0.xxx / max(g, 1e-4));
}

float3 Posterize(float3 c, float levels)
{
    levels = max(levels, 2.0);
    float3 q = floor(saturate(c) * (levels - 1.0) + 0.5) / (levels - 1.0);
    return saturate(q);
}

float Luma(float3 c)
{
    return dot(c, float3(0.299, 0.587, 0.114));
}

// Sobel magnitude on luma
float SobelEdge(Texture2D tex, SamplerState smp, float2 uv, float2 texel, float thickness)
{
    float2 t = texel * thickness;

    float l00 = Luma(tex.SampleLevel(smp, uv + float2(-t.x, -t.y), 0).rgb);
    float l10 = Luma(tex.SampleLevel(smp, uv + float2(0.0, -t.y), 0).rgb);
    float l20 = Luma(tex.SampleLevel(smp, uv + float2(t.x, -t.y), 0).rgb);

    float l01 = Luma(tex.SampleLevel(smp, uv + float2(-t.x, 0.0), 0).rgb);
    float l21 = Luma(tex.SampleLevel(smp, uv + float2(t.x, 0.0), 0).rgb);

    float l02 = Luma(tex.SampleLevel(smp, uv + float2(-t.x, t.y), 0).rgb);
    float l12 = Luma(tex.SampleLevel(smp, uv + float2(0.0, t.y), 0).rgb);
    float l22 = Luma(tex.SampleLevel(smp, uv + float2(t.x, t.y), 0).rgb);

    float gx = (-1.0 * l00) + (1.0 * l20) +
               (-2.0 * l01) + (2.0 * l21) +
               (-1.0 * l02) + (1.0 * l22);

    float gy = (-1.0 * l00) + (-2.0 * l10) + (-1.0 * l20) +
               (1.0 * l02) + (2.0 * l12) + (1.0 * l22);

    // Normalize a bit so threshold is intuitive;  edge ~0..1
    float g = sqrt(gx * gx + gy * gy);
    // Empirical scale so edges live ~0..1
    return saturate(g * 1.0);
}

float4 main(VSOut i) : SV_Target
{
    float2 res = max(iResolution.xy, 1.0.xx);
    float2 texel = 1.0 / res;

    // --- defaults ---
    float levels = (ExtraPram_1.x != 0.0) ? ExtraPram_1.x : 4.0;
    float contrastK = (ExtraPram_1.y != 0.0) ? ExtraPram_1.y : 1.0;
    float saturationK = (ExtraPram_1.z != 0.0) ? ExtraPram_1.z : 1.0;
    float gammaK = (ExtraPram_1.w != 0.0) ? ExtraPram_1.w : 1.0;

    float edgeThickness = (ExtraPram_2.x != 0.0) ? ExtraPram_2.x : 1.0;
    float edgeStrength = (ExtraPram_2.y != 0.0) ? ExtraPram_2.y : 1.5;
    float edgeThresh = (ExtraPram_2.z != 0.0) ? ExtraPram_2.z : 0.25;
    float edgeSoft = (ExtraPram_2.w != 0.0) ? ExtraPram_2.w : 0.10;

    float3 edgeColor = (ExtraPram_3.xyz != 0.0.xxx) ? ExtraPram_3.xyz : 0.0.xxx;
    float globalBlend = (ExtraPram_3.w != 0.0) ? ExtraPram_3.w : 1.0; // 1 = full toon, 0 = original

    // --- source color ---
    float3 srcCol = Src.Sample(Samp, i.uv).rgb;

    // --- tone ops before quantization (optional order) ---
    float3 pre = srcCol;
    pre = ApplyGamma(pre, max(gammaK, 1e-3)); // gamma space control
    pre = ApplyContrast(pre, contrastK); // expand/compress contrast
    pre = ApplySaturation(pre, saturationK); // boost/trim saturation

    // --- posterize ---
    float3 toon = Posterize(pre, levels);

    // --- edges ---
    float edgeMag = SobelEdge(Src, Samp, i.uv, texel, edgeThickness);
    // Soft threshold to get binary-ish lines but still anti-aliased
    float edgeMask = smoothstep(edgeThresh, edgeThresh + edgeSoft, edgeMag);
    edgeMask = saturate(edgeMask * edgeStrength);

    float3 outlined = lerp(toon, edgeColor, edgeMask);

    // --- global blend with original source (user-controlled) ---
    float3 outCol = lerp(srcCol, outlined, saturate(globalBlend));

    return float4(outCol, 1.0);
}
