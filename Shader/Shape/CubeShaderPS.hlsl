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
    float3 padding; // ensures 16-byte alignment (64 bytes total)
};

struct DIRECTIONAL_LIGHT_GPU_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;
    float3 Direction;
    float  SpecularPower;
};

struct SPOT_LIGHT_GPU_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;

    float3 Position;
    float  Range;

    float3 Direction;
    float  SpotAngle; // Cosine of outer cone angle

    float  SpecularPower;
    float3 Padding;
};

struct POINT_LIGHT_GPU_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;

    float3 Position;
    float  Range;

    float  SpecularPower;
    float3 Padding;
};

StructuredBuffer<DIRECTIONAL_LIGHT_GPU_DATA> gDirectionalLights : register(t0);
StructuredBuffer<SPOT_LIGHT_GPU_DATA> gSpotLights : register(t1);
StructuredBuffer<POINT_LIGHT_GPU_DATA> gPointLights: register(t2);

Texture2D gTexture             : register(t3);  // Base Albedo
Texture2D gTextureSecondary    : register(t4);  // Second Albedo
Texture2D gLightMapping        : register(t5);  // Baked light/shadow map
Texture2D gAlphaMapping        : register(t6);  // Transparency or merge style with Second Albedo
Texture2D gNormalMapping       : register(t7);  // Normal map
Texture2D gHeightMap           : register(t8);  // Height map for parallax/displacement
Texture2D gRoughnessMap        : register(t9);  // PBR Roughness
Texture2D gMetalnessMap        : register(t10); // PBR Metalness
Texture2D gAOMap               : register(t11); // Ambient Occlusion
Texture2D gSpecularMap         : register(t12); // Specular highlights
Texture2D gEmissiveMap         : register(t13); // Emissive/self-lighting
Texture2D gDisplacementMap     : register(t14); // Displacement map (optional from height)

SamplerState gSampler          : register(s0);


struct VSOutput
{
    float4 Position      : SV_POSITION;
    float2 Tex           : TEXCOORD0;
    float3 ViewDirection : TEXCOORD1;
    float3 WorldPos      : TEXCOORD2;
    float3x3 TBN         : TEXCOORD3;
};

// === Lighting Calculations ===
float3 ComputeDirectionalLight(DIRECTIONAL_LIGHT_GPU_DATA light, float3 N, float3 V, float3 baseColor)
{
    float3 L = normalize(-light.Direction);
    float3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0f);
    float NdotH = max(dot(N, H), 0.0f);
    float specIntensity = pow(NdotH, max(light.SpecularPower, 1.0f));

    float3 diffuse = light.DiffuseColor.rgb * baseColor * NdotL;
    float3 specular = light.SpecularColor.rgb * specIntensity;
    float3 ambient = light.AmbientColor.rgb * baseColor;

    return ambient + diffuse + specular;
}

float4 ComputeSpotLight(SPOT_LIGHT_GPU_DATA light, float3 normal, float3 fragPos, float3 viewDir)
{
    float3 lightDir = light.Position - fragPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);

    float NdotL = max(dot(normal, lightDir), 0.0f);

    // Attenuation by distance
    float attenuation = saturate(1.0f - (distance / light.Range));

    // Spotlight angle factor
    float spotCos = dot(-lightDir, normalize(light.Direction));
    float spotFactor = smoothstep(light.SpotAngle, light.SpotAngle + 0.05, spotCos);

    // Combine attenuation and spot factor
    float finalAtten = attenuation * spotFactor;

    float3 diffuse = light.DiffuseColor.rgb * NdotL * finalAtten;

    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), light.SpecularPower);
    float3 specular = light.SpecularColor.rgb * spec * finalAtten;

    return float4(diffuse + specular, 1.0f);
}

// ===================================
// === Point Light Computation ===
// ===================================

float3 ComputePointLight(POINT_LIGHT_GPU_DATA light, float3 N, float3 V, float3 worldPos, float3 baseColor)
{
    float3 lightDir = worldPos - light.Position;
    float dist = length(lightDir);
    float3 L = normalize(-lightDir); // From light to fragment

    float attenuation = saturate(1.0f - dist / light.Range);

    float3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.0f);
    float NdotH = max(dot(N, H), 0.0f);
    float specIntensity = pow(NdotH, max(light.SpecularPower, 1.0f));

    float3 diffuse = light.DiffuseColor.rgb * baseColor * NdotL;
    float3 specular = light.SpecularColor.rgb * specIntensity;
    float3 ambient = light.AmbientColor.rgb * baseColor;

    return (ambient + diffuse + specular) * attenuation;
}

float4 main(VSOutput input) : SV_TARGET
{
    if (bDebugLine == 1)
        return float4(0.0f, 1.0f, 0.0f, 1.0f);

    if (bTexture == 0)
        return float4(0, 0, 0, 0);

    float2 uv = input.Tex;

    // === Optional Parallax-style UV offset via height map ===
    if (bHeightMap == 1)
    {
        float height = gHeightMap.Sample(gSampler, uv).r;
        float2 viewOffset = input.ViewDirection.xy * height * 0.05; // optional scale
        uv += viewOffset;
    }

    float4 baseColor = gTexture.Sample(gSampler, uv);
    float alphaMapVal = 1.0f;

    if (bAlphaMap == 1)
        alphaMapVal = gAlphaMapping.Sample(gSampler, uv).r;

    if (bMultiTexturing == 1)
    {
        float4 secondaryColor = gTextureSecondary.Sample(gSampler, uv);
        baseColor.rgb = lerp(baseColor.rgb, secondaryColor.rgb, alphaMapVal);
        baseColor.a = max(baseColor.a, secondaryColor.a * alphaMapVal);
    }
    else
    {
        baseColor.a *= alphaMapVal;
    }

    // === Final normal calculation ===
    float3 N = normalize(input.TBN[2]);
    if (bNormalMap == 1)
    {
        float3 sampledNormal = gNormalMapping.Sample(gSampler, uv).rgb * 2.0 - 1.0;
        N = normalize(mul(sampledNormal, input.TBN));
    }

    float3 V = normalize(input.ViewDirection);
    float3 worldPos = input.WorldPos;
    float3 albedo = baseColor.rgb;

    // === Prepare material inputs ===
    float roughness = 0.5f;
    float metalness = 0.0f;
    float3 specularTint = float3(1.0f, 1.0f, 1.0f);
    float ao = 1.0f;

    if (bRoughnessMap == 1)
        roughness = gRoughnessMap.Sample(gSampler, uv).r;

    if (bMetalnessMap == 1)
        metalness = gMetalnessMap.Sample(gSampler, uv).r;

    if (bSpecularMap == 1)
        specularTint = gSpecularMap.Sample(gSampler, uv).rgb;

    if (bAOMap == 1)
        ao = gAOMap.Sample(gSampler, uv).r;

    // === Lighting ===
    float3 finalRGB = float3(0, 0, 0);

    for (int i = 0; i < DirectionalLightCount; ++i)
        finalRGB += ComputeDirectionalLight(gDirectionalLights[i], N, V, albedo);

    for (int i = 0; i < SpotLightCount; ++i)
        finalRGB += ComputeSpotLight(gSpotLights[i], N, worldPos, V).rgb;

    for (int i = 0; i < PointLightCount; ++i)
        finalRGB += ComputePointLight(gPointLights[i], N, V, worldPos, albedo).rgb;

    // Apply ambient occlusion
    finalRGB *= ao;

    // Apply lightmap (baked lighting)
    if (bLightMap == 1)
    {
        float3 lightMap = gLightMapping.Sample(gSampler, uv).rgb;
        finalRGB *= lightMap;
    }

    // Add emissive contribution
    if (bEmissiveMap == 1)
    {
        float3 emissive = gEmissiveMap.Sample(gSampler, uv).rgb;
        finalRGB += emissive;
    }

    // Final alpha output
    float finalAlpha = baseColor.a;
    if (AlphaValue >= 0.0f)
        finalAlpha *= clamp(AlphaValue, 0.1f, 1.0f);

    return float4(saturate(finalRGB), finalAlpha);
}
