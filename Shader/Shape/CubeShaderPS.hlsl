cbuffer LightMeta : register(b0)
{
    int DirectionalLightCount;
    int SpotLightCount;
    int PointLightCount;
    int bDebugLine;
    int bTexture;
    int bMultiTexturing; // 0 means no, 1 means yes
    int bLightMap;
    int bAlphaMap;
    float AlphaValue;
    int bNormalMap;
    float2 padding;
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

Texture2D gTexture : register(t3);
Texture2D gTextureSecondary  : register(t4);
Texture2D gLightMapping  : register(t5);
Texture2D gAlphaMapping  : register(t6);
Texture2D gNormalMapping: register(t7);
SamplerState gSampler : register(s0);

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
        return float4(0.0f, 1.0f, 0.0f, 1.0f); // Debug wireframe

    if (bTexture == 0)
        return float4(0, 0, 0, 0); // No texture = full transparent

    float4 baseColor = gTexture.Sample(gSampler, input.Tex);
    float alphaMapVal = 1.0f;

    if (bAlphaMap == 1)
        alphaMapVal = gAlphaMapping.Sample(gSampler, input.Tex).r;

    // === Multi-texturing logic ===
    if (bMultiTexturing == 1)
    {
        float4 secondaryColor = gTextureSecondary.Sample(gSampler, input.Tex);

        // Blend secondary based on alpha map value
        baseColor.rgb = lerp(baseColor.rgb, secondaryColor.rgb, alphaMapVal);
        baseColor.a = max(baseColor.a, secondaryColor.a * alphaMapVal);
    }
    else
    {
        baseColor.a *= alphaMapVal; // Even if no multi-texturing, alpha map matters
    }

    // === Normal Mapping ===
    float3 N = normalize(input.TBN[2]); // default world normal
    if (bNormalMap == 1)
    {
        float3 normalSample = gNormalMapping.Sample(gSampler, input.Tex).rgb * 2.0 - 1.0;
        N = normalize(mul(normalSample, input.TBN)); // transform from tangent to world
    }

    float3 V = normalize(input.ViewDirection);
    float3 albedo = baseColor.rgb;
    float3 worldPos = input.WorldPos;

    // === Lighting ===
    float3 finalRGB = float3(0, 0, 0);

    if (DirectionalLightCount + SpotLightCount + PointLightCount == 0)
        return float4(0, 0, 0, baseColor.a); // dark scene fallback

    for (int i = 0; i < DirectionalLightCount; ++i)
        finalRGB += ComputeDirectionalLight(gDirectionalLights[i], N, V, albedo);

    for (int i = 0; i < SpotLightCount; ++i)
        finalRGB += ComputeSpotLight(gSpotLights[i], N, worldPos, V).rgb;

    for (int i = 0; i < PointLightCount; ++i)
        finalRGB += ComputePointLight(gPointLights[i], N, V, worldPos, albedo).rgb;

    // === Light Map Modulation (optional baked lighting multiplier) ===
    if (bLightMap == 1)
    {
        float3 lightMap = gLightMapping.Sample(gSampler, input.Tex).rgb;
        finalRGB *= lightMap;
    }

    // === Final Alpha Control ===
    float finalAlpha = baseColor.a;
    if (AlphaValue >= 0.0f)
        finalAlpha *= clamp(AlphaValue, 0.1f, 1.0f); // user slider on top

    return float4(saturate(finalRGB), finalAlpha);
}
