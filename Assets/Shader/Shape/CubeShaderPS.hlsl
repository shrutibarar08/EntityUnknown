// ======================================================================================
// Constant Buffers & Resources
// ======================================================================================
cbuffer LightMeta : register(b0)
{
    int DirectionalLightCount;
    int SpotLightCount;
    int PointLightCount;
    int bDebugLine;

    int bTexture;
    int bMultiTexturing; // 0 = no, 1 = yes
    int bLightMap;
    int bAlphaMap;

    float AlphaValue;
    int bNormalMap;
    int bHeightMap;
    int bRoughnessMap;

    int bMetalnessMap;
    int bAOMap;
    int bSpecularMap;
    int bEmissiveMap;

    int bDisplacementMap;
    float3 padding; // alignment
};

struct DIRECTIONAL_LIGHT_GPU_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;
    float3 Direction;
    float SpecularPower;
    float4x4 ViewProjectMatrix;
};

struct SPOT_LIGHT_GPU_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;

    float3 Position;
    float Range;
    float3 Direction;
    float SpotAngleCosine;
    float SpecularPower;
    float3 _pad;

    float4x4 ViewProjectMatrix;
};

struct POINT_LIGHT_GPU_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;

    float3 Position;
    float Range;
    float SpecularPower;
    float3 _pad;

    float4x4 ViewProjectMatrix;
};

StructuredBuffer<DIRECTIONAL_LIGHT_GPU_DATA> gDirectionalLights : register(t0);
StructuredBuffer<SPOT_LIGHT_GPU_DATA> gSpotLights : register(t1);
StructuredBuffer<POINT_LIGHT_GPU_DATA> gPointLights : register(t2);

Texture2D gTexture : register(t3); // Base Albedo
Texture2D gTextureSecondary : register(t4); // Second Albedo
Texture2D gLightMapping : register(t5); // Baked light/shadow map
Texture2D gAlphaMapping : register(t6); // Blend/Opacity
Texture2D gNormalMapping : register(t7); // Normal map (TS)
Texture2D gHeightMap : register(t8); // Height (parallax)
Texture2D gRoughnessMap : register(t9); // Roughness
Texture2D gMetalnessMap : register(t10); // Metalness
Texture2D gAOMap : register(t11); // AO
Texture2D gSpecularMap : register(t12); // Specular tint
Texture2D gEmissiveMap : register(t13); // Emissive
Texture2D gDisplacementMap : register(t14); // (not used here)

Texture2DArray<float> gDirectionalShadowMaps : register(t15);
Texture2DArray<float> gPointLightShadowMaps : register(t16);
Texture2DArray<float> gSpotLightShadowMaps : register(t17);

SamplerState gSampler : register(s0);
SamplerComparisonState gShadowSampler : register(s1); // not used here

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 ViewDirection : TEXCOORD1; // world-space V
    float3 WorldPos : TEXCOORD2;
    float3x3 TBN : TEXCOORD3; // rows: T, B, N in WORLD space
};

// ======================================================================================
// Small utilities
// ======================================================================================
float3 SafeNormalize(float3 v)
{
    float len2 = dot(v, v);
    return (len2 > 1e-5f) ? v * rsqrt(len2) : float3(0, 0, 1);
}

float3 SafeNormalizeEps(float3 v, float eps)
{
    float len2 = dot(v, v);
    return (len2 > eps) ? v * rsqrt(len2) : float3(0, 0, 1);
}

float3x3 OrthonormalizeTBN(float3x3 tbn)
{
    float3 T = SafeNormalize(tbn[0]);
    float3 B = SafeNormalize(tbn[1]);
    float3 N = SafeNormalize(tbn[2]);

    // Gram-Schmidt
    T = SafeNormalize(T - N * dot(T, N));
    B = SafeNormalize(cross(N, T));
    return float3x3(T, B, N);
}

// Convert world-space vector to tangent space using TBN
float3 WorldToTangent(float3x3 TBN, float3 Vworld)
{
    // With TBN rows as T, B, N in world, TS = V * TBN (or mul(transpose(TBN), V))
    return mul(transpose(TBN), Vworld);
}

// ======================================================================================
// Parallax mapping (cheap offset, not steep parallax)
// ======================================================================================
float2 ParallaxOffsetUV(float2 uv, float3 Vworld, float3x3 TBN, float scale)
{
    float3 Vts = WorldToTangent(TBN, SafeNormalize(Vworld));
    float height = gHeightMap.Sample(gSampler, uv).r; // [0..1]
    float parallax = (height - 0.5f) * 2.0f; // center around 0
    float vz = max(abs(Vts.z), 0.1f); // avoid blow-up at grazing
    float2 offset = (Vts.xy / vz) * parallax * scale;
    return uv + offset;
}

// ======================================================================================
// Normal decoding
// ======================================================================================
float3 DecodeNormalTS(float2 uv)
{
    float3 n = gNormalMapping.Sample(gSampler, uv).xyz * 2.0f - 1.0f;
    return SafeNormalize(n);
}

float3 ToWorldFromTS(float3 nTS, float3x3 TBN)
{
    // world = TBN * nTS  (TBN rows are world-space basis)
    return SafeNormalize(mul(TBN, nTS));
}

// ======================================================================================
// Material parameter sampling
// ======================================================================================
struct MaterialParams
{
    float3 albedo;
    float alpha;
    float roughness;
    float metalness;
    float3 specularTint;
    float ao;
};

MaterialParams SampleMaterial(float2 uv)
{
    MaterialParams m;
    float4 base = gTexture.Sample(gSampler, uv);
    m.albedo = base.rgb;
    m.alpha = base.a;

    if (bAlphaMap == 1)
    {
        float alphaMap = gAlphaMapping.Sample(gSampler, uv).r;
        if (bMultiTexturing == 1)
        {
            float4 second = gTextureSecondary.Sample(gSampler, uv);
            m.albedo = lerp(m.albedo, second.rgb, alphaMap);
            m.alpha = max(m.alpha, second.a * alphaMap);
        }
        else
        {
            m.alpha *= alphaMap;
        }
    }

    m.roughness = (bRoughnessMap == 1) ? gRoughnessMap.Sample(gSampler, uv).r : 0.5f;
    m.metalness = (bMetalnessMap == 1) ? gMetalnessMap.Sample(gSampler, uv).r : 0.0f;
    m.specularTint = (bSpecularMap == 1) ? gSpecularMap.Sample(gSampler, uv).rgb : float3(1, 1, 1);
    m.ao = (bAOMap == 1) ? gAOMap.Sample(gSampler, uv).r : 1.0f;

    // Clamp to sane ranges
    m.roughness = saturate(m.roughness);
    m.metalness = saturate(m.metalness);
    m.alpha = saturate(m.alpha);
    m.ao = saturate(m.ao);
    return m;
}

// ======================================================================================
// Simple lighting lobes (Lambert + Blinn to match your look)
// ======================================================================================
struct ShadeInputs
{
    float3 N;
    float3 V;
    float3 albedo;
    float roughness;
    float metalness;
    float3 specularTint;
};

float3 DiffuseLambert(float3 albedo, float NdotL)
{
    // 1/pi omitted to preserve your existing look
    return albedo * NdotL;
}

float BlinnPhongSpec(float NdotH, float specPower)
{
    return pow(saturate(NdotH), max(specPower, 1.0f));
}

float3 ApplyMetalRough(float3 diffuse, float3 specular, float roughness, float metalness, float3 specTint)
{
    diffuse *= (1.0 - metalness);
    specular *= specTint * (1.0 - roughness);
    return diffuse + specular;
}

// ======================================================================================
// Light evaluators
// ======================================================================================
float3 ShadeDirectional(DIRECTIONAL_LIGHT_GPU_DATA Ld, ShadeInputs S)
{
    float3 L = SafeNormalize(-Ld.Direction);
    float3 H = SafeNormalize(S.V + L);
    float NdotL = saturate(dot(S.N, L));
    float NdotH = saturate(dot(S.N, H));

    float3 diff = Ld.DiffuseColor.rgb * DiffuseLambert(S.albedo, NdotL);
    float3 spec = Ld.SpecularColor.rgb * BlinnPhongSpec(NdotH, Ld.SpecularPower);

    float3 color = ApplyMetalRough(diff, spec, S.roughness, S.metalness, S.specularTint);
    color += Ld.AmbientColor.rgb * S.albedo;
    return color;
}

float SpotAttenuation(SPOT_LIGHT_GPU_DATA Ls, float3 worldPos, out float3 Ldir)
{
    float3 toLight = Ls.Position - worldPos;
    float dist = length(toLight);
    Ldir = (dist > 1e-5f) ? toLight / dist : float3(0, 0, 1);

    float rangeAtten = saturate(1.0f - dist / max(Ls.Range, 1e-3f));

    float cosAng = dot(-Ldir, SafeNormalize(Ls.Direction));
    // Small penumbra slope for smooth edge
    float spot = smoothstep(Ls.SpotAngleCosine, saturate(Ls.SpotAngleCosine + 0.05f), cosAng);

    return rangeAtten * spot;
}

float3 ShadeSpot(SPOT_LIGHT_GPU_DATA Ls, ShadeInputs base, float3 worldPos)
{
    float3 L;
    float atten = SpotAttenuation(Ls, worldPos, L);

    float3 H = SafeNormalize(base.V + L);
    float NdotL = saturate(dot(base.N, L));
    float NdotH = saturate(dot(base.N, H));

    float3 diff = Ls.DiffuseColor.rgb * DiffuseLambert(base.albedo, NdotL);
    float3 spec = Ls.SpecularColor.rgb * BlinnPhongSpec(NdotH, Ls.SpecularPower);

    float3 color = ApplyMetalRough(diff, spec, base.roughness, base.metalness, base.specularTint);
    color += Ls.AmbientColor.rgb * base.albedo;
    return color * atten;
}

float RangeAttenuationPoint(POINT_LIGHT_GPU_DATA Lp, float3 worldPos, out float3 L)
{
    float3 toLight = Lp.Position - worldPos;
    float dist = length(toLight);
    L = (dist > 1e-5f) ? toLight / dist : float3(0, 0, 1);
    return saturate(1.0f - dist / max(Lp.Range, 1e-3f));
}

float3 ShadePoint(POINT_LIGHT_GPU_DATA Lp, ShadeInputs base, float3 worldPos)
{
    float3 L;
    float atten = RangeAttenuationPoint(Lp, worldPos, L);

    float3 H = SafeNormalize(base.V + L);
    float NdotL = saturate(dot(base.N, L));
    float NdotH = saturate(dot(base.N, H));

    float3 diff = Lp.DiffuseColor.rgb * DiffuseLambert(base.albedo, NdotL);
    float3 spec = Lp.SpecularColor.rgb * BlinnPhongSpec(NdotH, Lp.SpecularPower);

    float3 color = ApplyMetalRough(diff, spec, base.roughness, base.metalness, base.specularTint);
    color += Lp.AmbientColor.rgb * base.albedo;
    return color * atten;
}

// ======================================================================================
// Main PS
// ======================================================================================
float4 main(VSOutput IN) : SV_TARGET
{
    if (bDebugLine == 1)
        return float4(0, 1, 0, 1);
    if (bTexture == 0)
        return float4(0, 0, 0, 0);

    // TBN safety
    float3x3 TBN = OrthonormalizeTBN(IN.TBN);

    // Parallax (cheap)
    float2 uv = IN.Tex;
    if (bHeightMap == 1)
    {
        uv = ParallaxOffsetUV(uv, IN.ViewDirection, TBN, /*scale*/0.05f);
    }

    // Material
    MaterialParams M = SampleMaterial(uv);

    // Normal (world space)
    float3 N = TBN[2];
    if (bNormalMap == 1)
    {
        float3 nTS = DecodeNormalTS(uv);
        N = ToWorldFromTS(nTS, TBN);
    }
    float3 V = SafeNormalize(IN.ViewDirection);

    // Accumulate lighting
    float3 finalRGB = float3(0, 0, 0);

    // Directional
    [loop]
    for (int i = 0; i < DirectionalLightCount; ++i)
    {
        ShadeInputs S = { N, V, M.albedo, M.roughness, M.metalness, M.specularTint };
        finalRGB += ShadeDirectional(gDirectionalLights[i], S);
    }

    // Spot
    [loop]
    for (int i = 0; i < SpotLightCount; ++i)
    {
        ShadeInputs S = { N, V, M.albedo, M.roughness, M.metalness, M.specularTint };
        finalRGB += ShadeSpot(gSpotLights[i], S, IN.WorldPos);
    }

    // Point
    [loop]
    for (int i = 0; i < PointLightCount; ++i)
    {
        ShadeInputs S = { N, V, M.albedo, M.roughness, M.metalness, M.specularTint };
        finalRGB += ShadePoint(gPointLights[i], S, IN.WorldPos);
    }

    // AO + LightMap + Emissive
    finalRGB *= M.ao;

    if (bLightMap == 1)
    {
        float3 lmap = gLightMapping.Sample(gSampler, uv).rgb;
        finalRGB *= lmap;
    }

    if (bEmissiveMap == 1)
    {
        finalRGB += gEmissiveMap.Sample(gSampler, uv).rgb;
    }

    // Alpha
    float alpha = M.alpha;
    if (AlphaValue >= 0.0f)
        alpha *= clamp(AlphaValue, 0.1f, 1.0f);

    return float4(saturate(finalRGB), saturate(alpha));
}
