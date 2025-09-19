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

    // Param mapping for snow:
    // ExtraPram_1: x = density (0.5..3), y = flakeSizePx (1..5),
    //              z = windX_px_per_sec, w = windY_px_per_sec
    // ExtraPram_2: x = streakLengthPx (0..20), y = layerParallax (0..1),
    //              z = turbulence (0..~5),     w = globalBlend (0..1)
    // ExtraPram_3: x = tint.r, y = tint.g, z = tint.b, w = opacity (0..1)
    float4 ExtraPram_1;
    float4 ExtraPram_2;
    float4 ExtraPram_3;
};

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float Hash11(float n)
{
    return frac(sin(n) * 43758.5453123);
}
float Hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 34.345);
    return frac(p.x * p.y);
}
float2 Hash22(float2 p)
{
    float n = sin(dot(p, float2(41.0, 289.0))) * 43758.5453;
    return frac(float2(n, n * 1.2154));
}
float2 Rotate2(float2 v, float a)
{
    float s = sin(a), c = cos(a);
    return float2(c * v.x - s * v.y, s * v.x + c * v.y);
}

// Distance to oriented segment (for streak flakes)
float LineDist(float2 p, float2 a, float2 b)
{
    float2 pa = p - a;
    float2 ba = b - a;
    float h = saturate(dot(pa, ba) / dot(ba, ba));
    return length(pa - ba * h);
}

// Soft round flake mask
float RoundFlake(float2 d, float radiusPx)
{
    float r = length(d);
    float t = saturate(1.0 - r / max(radiusPx, 1e-5));
    return t * t; // slightly sharper center
}

// One procedural snow layer
float LayerSnow(
    float2 uv, float2 res, float time,
    float2 windPxPerSec, float cellPx,
    float flakeSizePx, float streakLenPx,
    float turbulence, float layerId,
    out float3 accumTint)
{
    float2 p = uv * res;
    float2 cs = float2(cellPx, cellPx);
    float2 gid = floor(p / cs);

    float alphaAccum = 0.0;
    accumTint = 0.0.xxx;

    // Layer-specific velocity (wind + gravity)
    float2 gravity = float2(0.0, 120.0 + layerId * 40.0); // px/s
    float2 vel = (windPxPerSec + gravity);

    // Slight per-cell speed variance
    float speedMul = lerp(0.55, 1.35, frac(Hash21(gid + layerId)));
    vel *= speedMul;

    [unroll]
    for (int oy = -1; oy <= 1; ++oy)
    {
        [unroll]
        for (int ox = -1; ox <= 1; ++ox)
        {
            float2 ng = gid + float2(ox, oy);
            float2 h2 = Hash22(ng + layerId * 131.0);
            float rnd = Hash21(ng + layerId * 997.0);

            // Base spawn inside cell
            float2 baseInCell = (h2 - 0.5) * (cs - flakeSizePx * 2.0);

            float2 start = (ng * cs) + baseInCell;

            // Toroidal wrap in a 4x4 cell area to recycle flakes cheaply
            float2 pos = start + fmod(vel * time + 4.0 * cs, 4.0 * cs) - 2.0 * cs;

            // Tiny wobble/turbulence
            float twA = (rnd * 6.28318 + time * (0.3 + 0.2 * layerId));
            float2 wob = Rotate2(float2(0.0, 1.0), twA) * (turbulence * (3.0 + layerId * 2.0));
            pos += wob;

            float2 d = (p - pos);

            float size = flakeSizePx * lerp(0.7, 1.4, rnd);

            // Round flake
            float aRound = RoundFlake(d, size);

            // Streak along velocity direction
            float2 dir = normalize(vel + 1e-3);
            float2 a = pos - dir * streakLenPx * lerp(0.3, 1.0, rnd);
            float2 b = pos + dir * streakLenPx * lerp(0.3, 1.0, rnd);
            float thickness = max(1.0, size * 0.45);
            float dLine = LineDist(p, a, b);
            float aLine = saturate(1.0 - dLine / thickness);

            // Some flakes are dots, some streaks
            float shapeSel = step(0.5, rnd);
            float aFlake = lerp(aRound, aLine, shapeSel);

            // Subtle twinkle
            float twinkle = 0.8 + 0.2 * sin(time * (2.0 + rnd * 3.0) + rnd * 6.28318);

            float2 ndc = p / res * 2.0 - 1.0;
            float edgeSoft = saturate(1.0 - 0.08 * dot(ndc, ndc));

            float contrib = smoothstep(0.0, 1.0, aFlake) * twinkle * edgeSoft;

            alphaAccum += contrib;
        }
    }

    alphaAccum *= 0.08;

    accumTint = 1.0.xxx;

    return saturate(alphaAccum);
}

float4 main(VSOut i) : SV_Target
{
    float2 res = max(iResolution.xy, 1.0.xx);
    float time = iTime;

    // Unpack w/ sane defaults
    float density = (ExtraPram_1.x != 0.0) ? ExtraPram_1.x : 1.0;
    float flakeSizePx = (ExtraPram_1.y != 0.0) ? ExtraPram_1.y : 2.0;
    float2 windPxPerSec = float2(ExtraPram_1.z, ExtraPram_1.w);
    if (all(windPxPerSec == 0.0))
        windPxPerSec = float2(25.0, 0.0);

    float streakLenPx = (ExtraPram_2.x != 0.0) ? ExtraPram_2.x : 8.0;
    float layerParallax = (ExtraPram_2.y != 0.0) ? ExtraPram_2.y : 0.65;
    float turbulence = (ExtraPram_2.z != 0.0) ? ExtraPram_2.z : 2.0;
    float globalBlend = (ExtraPram_2.w != 0.0) ? ExtraPram_2.w : 1.0;

    float3 snowTint = (ExtraPram_3.xyz != 0.0.xxx) ? ExtraPram_3.xyz : 1.0.xxx;
    float snowOpacity = (ExtraPram_3.w != 0.0) ? ExtraPram_3.w : 0.85;

    float3 base = Src.Sample(Samp, i.uv).rgb;

    // Layer count from density
    int layers = (int) clamp(round(2.0 + density), 2.0, 5.0);

    float alphaAll = 0.0;
    float3 tintAccum = 0.0.xxx;

    // Slight parallax: foreground layers move faster
    [loop]
    for (int L = 0; L < layers; ++L)
    {
        float lf = (float) L / max(1.0, (float) (layers - 1));
        float layerScale = lerp(1.0 - 0.25 * layerParallax, 1.0 + 0.35 * layerParallax, lf);
        float2 luv = (i.uv - 0.5) * layerScale + 0.5;

        float sizeL = flakeSizePx * lerp(0.8, 1.6, 1.0 - lf);
        float cellPx = lerp(48.0, 26.0, density) * lerp(1.4, 0.8, 1.0 - lf);

        float3 layerTint;
        float aL = LayerSnow(
            luv, res, time,
            windPxPerSec * lerp(0.7, 1.3, 1.0 - lf),
            cellPx, sizeL,
            streakLenPx * lerp(0.6, 1.4, 1.0 - lf),
            turbulence, (float) L + 0.37, layerTint
        );

        aL *= lerp(0.6, 1.0, 1.0 - lf);

        alphaAll += aL;
        tintAccum += layerTint * aL;
    }

    float3 snowColor = (alphaAll > 1e-4) ? (tintAccum / alphaAll) : 1.0.xxx;
    snowColor *= snowTint;

    float snowA = saturate(alphaAll) * globalBlend * snowOpacity;

    float3 outCol = lerp(base, snowColor, snowA);

    return float4(outCol, 1.0);
}
