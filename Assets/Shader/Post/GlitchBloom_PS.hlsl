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

    // Params:
    // ExtraPram_1: x = bloomThreshold, y = bloomIntensity, z = blurRadiusPx, w = barrelAmount
    // ExtraPram_2: x = vignetteStrength, y = vignettePower,   z = grainStrength, w = grainScale
    // ExtraPram_3: x = glitchToggle(>=0.5), y = glitchIntensity, z = chromaSplitPx, w = scanlineStrength
    float4 ExtraPram_1;
    float4 ExtraPram_2;
    float4 ExtraPram_3;
};

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float3 Saturation(float3 c, float s)
{
    float g = dot(c, float3(0.299, 0.587, 0.114));
    return lerp(float3(g, g, g), c, s);
}

float Hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 34.345);
    return frac(p.x * p.y);
}

float Noise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    float a = Hash21(i);
    float b = Hash21(i + float2(1, 0));
    float c = Hash21(i + float2(0, 1));
    float d = Hash21(i + float2(1, 1));
    float2 u = f * f * (3.0 - 2.0 * f);
    return lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
}

// Barrel distortion (radial)
float2 BarrelUV(float2 uv, float amount)
{
    if (abs(amount) < 1e-4)
        return uv;
    float2 p = uv * 2.0 - 1.0;
    float r2 = dot(p, p);
    float k = amount;
    float2 pd = p * (1.0 + k * r2);
    return (pd * 0.5 + 0.5);
}

static const float w0 = 0.227027f;
static const float w1 = 0.1945946f;
static const float w2 = 0.1216216f;
static const float w3 = 0.054054f;
static const float w4 = 0.016216f;

float3 BlurDir(Texture2D tex, SamplerState s, float2 uv, float2 dir)
{
    float3 c = tex.Sample(s, uv).rgb * w0;
    c += tex.Sample(s, uv + dir * 1.0).rgb * w1;
    c += tex.Sample(s, uv - dir * 1.0).rgb * w1;
    c += tex.Sample(s, uv + dir * 2.0).rgb * w2;
    c += tex.Sample(s, uv - dir * 2.0).rgb * w2;
    c += tex.Sample(s, uv + dir * 3.0).rgb * w3;
    c += tex.Sample(s, uv - dir * 3.0).rgb * w3;
    c += tex.Sample(s, uv + dir * 4.0).rgb * w4;
    c += tex.Sample(s, uv - dir * 4.0).rgb * w4;
    return c;
}

// --- PS ---
float4 main(VSOut i) : SV_Target
{
    float2 res = max(iResolution.xy, 1.0.xx);
    float2 inv = 1.0 / res;

    // Unpack params with sane defaults
    float bloomThreshold = (ExtraPram_1.x != 0.0) ? ExtraPram_1.x : 1.0;
    float bloomIntensity = (ExtraPram_1.y != 0.0) ? ExtraPram_1.y : 0.9;
    float blurRadiusPx = (ExtraPram_1.z != 0.0) ? ExtraPram_1.z : 3.0;
    float barrelAmount = ExtraPram_1.w; // 0 = off, 0.05..0.15 nice

    float vignetteStr = (ExtraPram_2.x != 0.0) ? ExtraPram_2.x : 0.25;
    float vignettePow = (ExtraPram_2.y != 0.0) ? ExtraPram_2.y : 2.0;
    float grainStrength = (ExtraPram_2.z != 0.0) ? ExtraPram_2.z : 0.035;
    float grainScale = (ExtraPram_2.w != 0.0) ? ExtraPram_2.w : 1.75;

    bool doGlitch = (ExtraPram_3.x >= 0.5);
    float glitchInt = (ExtraPram_3.y != 0.0) ? ExtraPram_3.y : 0.6;
    float chromaSplitPx = (ExtraPram_3.z != 0.0) ? ExtraPram_3.z : 1.5;
    float scanStrength = (ExtraPram_3.w != 0.0) ? ExtraPram_3.w : 0.15;

    float2 uv = BarrelUV(i.uv, barrelAmount);

    if (doGlitch)
    {
        float bandN = floor(uv.y * res.y / 8.0);
        float bandPhase = Hash21(float2(bandN, iTime * 0.5)) * 6.28318;
        float bandAmp = glitchInt * 1.5 * inv.x * (0.5 + 0.5 * sin(iTime * 2.3 + bandPhase));
        uv.x += bandAmp * sin(uv.y * res.y * 0.25 + iTime * 20.0 + bandPhase);
        // Occasional vertical jump
        float jump = step(0.98, Hash21(float2(iTime * 0.2, bandN))) * glitchInt * 2.0 * inv.y;
        uv.y = saturate(uv.y + jump);
    }

    float2 center = float2(0.5, 0.5);
    float2 dirC = normalize(uv - center + 1e-6);
    float2 offC = dirC * chromaSplitPx * inv;

    float r = Src.Sample(Samp, uv + offC).r;
    float g = Src.Sample(Samp, uv).g;
    float b = Src.Sample(Samp, uv - offC).b;
    float3 base = float3(r, g, b);

    // Bright-pass
    float lum = dot(base, float3(0.2126, 0.7152, 0.0722));
    float3 bright = max(base - bloomThreshold, 0.0) * (1.0 / max(1e-4, 1.0 - bloomThreshold));
    bright *= step(bloomThreshold, lum);

    float px = blurRadiusPx;
    float2 dirX = float2(px, 0) * inv;
    float2 dirY = float2(0, px) * inv;

    float3 blurX = BlurDir(Src, Samp, uv, dirX);
    float3 blurY = BlurDir(Src, Samp, uv, dirY);

    float3 bloom = (blurX + blurY) * 0.5 * bloomIntensity;
    bloom *= saturate(dot(bright, 0.333));

    float2 grainUV = uv * res / max(1.0, 64.0 / grainScale);
    float g0 = Noise(grainUV + iTime * 1.37);
    float g1 = Noise(grainUV * 1.7 - iTime * 0.73);
    float grain = (g0 * 0.6 + g1 * 0.4) * 2.0 - 1.0;

    float scan = 1.0 - scanStrength * 0.5 + 0.5 * scanStrength * sin(uv.y * res.y * 3.14159);

    float2 p = uv * 2.0 - 1.0;
    float vig = pow(saturate(1.0 - dot(p, p)), vignettePow);
    float vignette = lerp(1.0 - vignetteStr, 1.0, vig);

    float3 color = base + bloom;
    color = Saturation(color, 1.06);
    color *= scan;
    color *= vignette;
    color += grainStrength * grain;

    return float4(color, 1.0);
}
