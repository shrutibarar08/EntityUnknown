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

    // ExtraPram_1: x=bloomThreshold, y=bloomIntensity, z=vignetteStrength, w=vignetteRoundness
    // ExtraPram_2: x=chromAbAmountPx, y=chromAbCurve, z=grainAmount, w=grainResponse
    // ExtraPram_3: xyz=tintColor, w=globalBlend
    float4 ExtraPram_1;
    float4 ExtraPram_2;
    float4 ExtraPram_3;
};

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// --------------------------------------------------------------
// Utils
// --------------------------------------------------------------
float Luma(float3 c)
{
    return dot(c, float3(0.299, 0.587, 0.114));
}

float3 ACESApprox(float3 x)   // simple filmic-ish toe/shoulder (works fine in LDR too)
{
    // Narkowicz-ACES approximation (safe even if src already LDR)
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Fast hash-based noise (temporal) — stable per-pixel with tiny time jitter
float Hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 34.345);
    return frac(p.x * p.y);
}

// 9-tap Gaussian-ish weights
float gW[9] =
{
    0.05, 0.09, 0.12, 0.09, 0.05,
               0.09, 0.16, 0.09, 0.12
}; // hand-tuned for a soft look

// --------------------------------------------------------------
// Cheap Bloom (bright extract + 9 taps around uv)
// --------------------------------------------------------------
float3 Bloom9(Texture2D tex, SamplerState smp, float2 uv, float2 texel, float threshold, float intensity)
{
    // bright-pass (soft knee)
    float3 c0 = tex.SampleLevel(smp, uv, 0).rgb;
    float l0 = Luma(c0);
    float w0 = saturate((l0 - threshold) / max(1e-4, (1.0 - threshold)));
    float3 b0 = c0 * w0;

    // ring-ish 9 taps (cross + diagonals); texel controls footprint
    float2 offs[8] =
    {
        float2(-1, 0), float2(1, 0),
        float2(0, -1), float2(0, 1),
        float2(-1, -1), float2(1, -1),
        float2(-1, 1), float2(1, 1)
    };

    float3 acc = b0 * gW[0];
    [unroll]
    for (int i = 0; i < 8; i++)
    {
        float3 c = tex.SampleLevel(smp, uv + offs[i] * texel * 1.5, 0).rgb;
        float l = Luma(c);
        float w = saturate((l - threshold) / max(1e-4, (1.0 - threshold)));
        acc += c * w * gW[i + 1];
    }

    return acc * intensity;
}

// --------------------------------------------------------------
// Chromatic Aberration (barrel-like radial shift per channel)
// amountPx in pixels; curve >=1 increases radial distortion
// --------------------------------------------------------------
float3 ChromAb(Texture2D tex, SamplerState smp, float2 uv, float2 res, float amountPx, float curve)
{
    float2 center = 0.5.xx;
    float2 d = uv - center;
    float r = pow(saturate(length(d) * 2.0), curve); // 0..~1 across screen
    float2 px = amountPx / max(res, 1.0.xx); // pixel->uv

    float2 uvR = uv + d * r * px; // red further outward
    float2 uvB = uv - d * r * px; // blue inward

    float rC = tex.SampleLevel(smp, uvR, 0).r;
    float gC = tex.SampleLevel(smp, uv, 0).g;
    float bC = tex.SampleLevel(smp, uvB, 0).b;
    return float3(rC, gC, bC);
}

// --------------------------------------------------------------
// Vignette (roundness in [0..1], 1=round, lower -> more rectangular)
// strength ~0..2 typical
// --------------------------------------------------------------
float Vignette(float2 uv, float strength, float roundness)
{
    float2 p = uv * 2.0 - 1.0; // -1..1
    float2 q = abs(p);
    q = pow(q, 1.0 + (1.0 - saturate(roundness)) * 2.0); // adjust shape
    float d = length(q);
    float v = 1.0 - saturate((d - 0.75) * (strength * 1.5)); // start falloff ~0.75 radius
    return v;
}

// --------------------------------------------------------------
// Film Grain (temporal, luminance-shaped)
// response >1 brightens grain in highlights; <1 pushes to shadows
// --------------------------------------------------------------
float3 FilmGrain(float2 uv, float2 res, float amount, float response)
{
    // random per pixel with mild time shift to avoid crawling
    float n = Hash21(floor(uv * res) + frac(iTime * 60.0));
    // remap via response; keep neutral mean ~0
    float g = (pow(n, response) - 0.5) * 2.0; // -1..1-ish
    return amount * g.xxx;
}

// --------------------------------------------------------------
// Main
// --------------------------------------------------------------
float4 main(VSOut i) : SV_Target
{
    float2 res = max(iResolution.xy, 1.0.xx);
    float2 texel = 1.0 / res;

    // --- read params with sane defaults ---
    float bloomThreshold = (ExtraPram_1.x != 0) ? ExtraPram_1.x : 1.0;
    float bloomIntensity = (ExtraPram_1.y != 0) ? ExtraPram_1.y : 0.85;
    float vignetteStrength = (ExtraPram_1.z != 0) ? ExtraPram_1.z : 0.25;
    float vignetteRound = (ExtraPram_1.w != 0) ? ExtraPram_1.w : 0.75;

    float chromAbPx = (ExtraPram_2.x != 0) ? ExtraPram_2.x : 1.0;
    float chromAbCurve = (ExtraPram_2.y != 0) ? ExtraPram_2.y : 1.5;
    float grainAmount = (ExtraPram_2.z != 0) ? ExtraPram_2.z : 0.04;
    float grainResponse = (ExtraPram_2.w != 0) ? ExtraPram_2.w : 1.0;

    float3 tintColor = (any(ExtraPram_3.xyz != 0)) ? ExtraPram_3.xyz : 1.0.xxx;
    float globalBlend = (ExtraPram_3.w != 0) ? ExtraPram_3.w : 1.0;

    // --- base sample ---
    float3 base = Src.Sample(Samp, i.uv).rgb;

    // --- cheap bloom ---
    float3 bloom = Bloom9(Src, Samp, i.uv, texel, bloomThreshold, bloomIntensity);

    // --- chromatic aberration (on the combined image for punch) ---
    float3 cab = ChromAb(Src, Samp, i.uv, res, chromAbPx, chromAbCurve);

    // combine: base + bloom, then mix with chromab a bit
    float3 col = base + bloom;
    col = lerp(col, cab, 0.25); // subtle CA mix to avoid color fringing overdose

    // --- vignette ---
    float v = Vignette(i.uv, vignetteStrength, vignetteRound);
    col *= v;

    // --- tint & mild ACES-ish curve to sweeten highlights ---
    col = ACESApprox(col * tintColor);

    // --- film grain (applied in linear-ish space) ---
    col += FilmGrain(i.uv, res, grainAmount, max(grainResponse, 1e-3));

    // --- final blend with original source ---
    float3 src = Src.Sample(Samp, i.uv).rgb;
    float3 outCol = lerp(src, col, saturate(globalBlend));

    return float4(saturate(outCol), 1.0);
}
