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

    float4 ExtraPram_1;
    float4 ExtraPram_2;
    float4 ExtraPram_3;
};

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// --- helpers ---
float3 HueShift(float3 color, float t)
{
    float a = t * 6.2831853;
    float3 k = float3(0.57735, 0.57735, 0.57735);
    float c = cos(a), s = sin(a);
    // Rodrigues' rotation formula around axis k
    return color * c + cross(k, color) * s + k * dot(k, color) * (1.0 - c);
}

float3 Saturation(float3 c, float s)
{
    float g = dot(c, float3(0.299, 0.587, 0.114));
    return lerp(float3(g, g, g), c, s);
}

float3 Posterize(float3 c, float steps)
{
    return floor(c * steps + 0.5) / steps;
}

// --- visible test PS ---
float4 main(VSOut i) : SV_Target
{
    float2 res = max(iResolution.xy, 1.0.xx);
    float2 inv = 1.0 / res;

    float chromaPx = (ExtraPram_1.x != 0.0) ? ExtraPram_1.x : 3.0; // pixels to offset R/B
    float vignetteP = (ExtraPram_1.y != 0.0) ? ExtraPram_1.y : 1.8; // vignette power
    float wobbleAmp = (ExtraPram_1.z != 0.0) ? ExtraPram_1.z : 6.0; // px amplitude
    float scanStrength = (ExtraPram_1.w != 0.0) ? ExtraPram_1.w : 0.25; // 0..1

    float hueShift = (ExtraPram_2.x != 0.0) ? ExtraPram_2.x : 0.08; // 0..1
    float satBoost = (ExtraPram_2.y != 0.0) ? ExtraPram_2.y : 1.35; // 1=no change
    float brightness = (ExtraPram_2.z != 0.0) ? ExtraPram_2.z : 1.05; // 1=no change

    bool doPoster = (ExtraPram_3.x >= 0.5);
    float posterSteps = (ExtraPram_3.y != 0.0) ? ExtraPram_3.y : 5.0;

    // Time-wobble UV (horizontal sine, vertical ripple)
    float wobbleX = sin(i.uv.y * 24.0 + iTime * 3.0) * wobbleAmp * inv.x;
    float wobbleY = sin(i.uv.x * 18.0 + iTime * 2.0) * wobbleAmp * inv.y;
    float2 uv = i.uv + float2(wobbleX, wobbleY);

    // Chromatic aberration outward from center
    float2 center = float2(0.5, 0.5);
    float2 dir = normalize(uv - center + 1e-6);
    float2 off = dir * chromaPx * inv;

    float r = Src.Sample(Samp, uv + off).r;
    float g = Src.Sample(Samp, uv).g;
    float b = Src.Sample(Samp, uv - off).b;
    float3 col = float3(r, g, b);

    // Vignette (chunky)
    float2 p = uv * 2.0 - 1.0;
    float v = pow(saturate(1.0 - dot(p, p)), vignetteP);
    col *= v;

    // Scanlines
    float scan = 1.0 - scanStrength * 0.5 + 0.5 * scanStrength * sin(uv.y * res.y * 3.14159);
    col *= scan;

    // Hue shift + saturation + brightness
    col = HueShift(col, hueShift);
    col = Saturation(col, satBoost);
    col *= brightness;

    // posterize
    if (doPoster)
        col = Posterize(col, max(posterSteps, 1.0));

    return float4(col, 1.0);
}
