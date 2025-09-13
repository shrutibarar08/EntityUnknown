// Post.hlsl
Texture2D g_Src : register(t0);
SamplerState g_Sam : register(s0);

cbuffer PostCB : register(b0)
{
    float2 g_InvTexel; // 1/width, 1/height
    float2 _pad_;
}

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOut VS_Fullscreen(uint id : SV_VertexID)
{
    float2 pos = float2((id == 2) ? 3.0 : -1.0,
                        (id == 1) ? 3.0 : -1.0);
    VSOut o;
    o.pos = float4(pos, 0, 1);
    // flip Y so texture isn't upside-down in D3D
    o.uv = pos * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    return o;
}

// =====================
// A) Big Gaussian 5x5
// =====================
float4 PS_BigBlur(VSOut i) : SV_Target
{
    float2 t = g_InvTexel;
    // 5x5 separable weights (sigma 1.4), normalized
    const float w[5] = { 0.06136, 0.24477, 0.38774, 0.24477, 0.06136 };

    float3 acc = 0;
    // horizontal
    [unroll]
    for (int x = -2; x <= 2; ++x)
        acc += g_Src.Sample(g_Sam, i.uv + float2(x, 0) * t).rgb * w[x + 2];

    // vertical blur of the horizontal result (approximation via sampling source again)
    float3 blur = 0;
    [unroll]
    for (int y = -2; y <= 2; ++y)
        blur += g_Src.Sample(g_Sam, i.uv + float2(0, y) * t).rgb * w[y + 2];

    // mix both for extra oomph
    float3 col = (acc + blur) * 0.5;
    return float4(col, 1);
}

// =====================
// B) Vintage w/ “VHS”
// =====================
float randhash(float2 p)
{
    // cheap static grain
    return frac(sin(dot(p, float2(12.9898, 78.233))) * 43758.5453);
}

float4 PS_VintagePunch(VSOut i) : SV_Target
{
    float2 t = g_InvTexel;

    // Chromatic aberration: separate R/B outwards from center
    float2 center = float2(0.5, 0.5);
    float2 dir = normalize(i.uv - center + 1e-6);
    float2 off = dir * 2.0 * t; // ~2 px shift

    float r = g_Src.Sample(g_Sam, i.uv + off).r;
    float g = g_Src.Sample(g_Sam, i.uv).g;
    float b = g_Src.Sample(g_Sam, i.uv - off).b;
    float3 c = float3(r, g, b);

    // Sepia
    float3 sepia;
    sepia.r = dot(c, float3(0.393, 0.769, 0.189));
    sepia.g = dot(c, float3(0.349, 0.686, 0.168));
    sepia.b = dot(c, float3(0.272, 0.534, 0.131));

    // Strong vignette
    float2 p = i.uv * 2 - 1;
    float v = pow(saturate(1 - dot(p, p)), 1.6); // heavier falloff
    sepia *= v;

    // Scanlines
    float resY = 1.0 / t.y;
    float scan = 0.90 + 0.10 * sin(i.uv.y * resY * 3.14159);
    sepia *= scan;

    // Tiny grain
    sepia += (randhash(i.uv * float2(resY, 1.0 / t.x)) - 0.5) * 0.02;

    return float4(sepia, 1);
}

// =====================
// C) Pixelate (8x8)
// =====================
float4 PS_Pixelate8(VSOut i) : SV_Target
{
    float2 cell = 8.0 * g_InvTexel; // 8x8 pixels per block
    float2 uvp = floor(i.uv / cell) * cell + cell * 0.5; // sample center of block
    float3 col = g_Src.Sample(g_Sam, uvp).rgb;
    return float4(col, 1);
}

float4 PS_AllWhite(VSOut i) : SV_Target
{
    return float4(1, 1, 1, 1);
}
