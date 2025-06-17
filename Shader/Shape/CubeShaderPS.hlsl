cbuffer LightMeta : register(b0)
{
    int DirectionalLightCount;
    int SpotLightCount;
    int PointLightCount;
    int DebugLine;
    int Texture;
    int MultiTexturing; // 0 means no, 1 means yes
    int NormalMap;
    float padding;
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
Texture2D gTextureSecondary  : register(t4); // Optional 2nd
Texture2D gNormalMap  : register(t5);
SamplerState gSampler : register(s0);

struct VSOutput
{
    float4 Position       : SV_POSITION;
    float2 Tex            : TEXCOORD0;
    float3 Normal         : TEXCOORD1;
    float3 viewDirection  : TEXCOORD2;
    float3 WorldPos       : TEXCOORD3;
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
    if (DebugLine == 1)
        return float4(0.0f, 1.0f, 0.0f, 1.0f); // Debug wireframe green

    if (Texture == 0)
        return float4(0.0f, 0.0f, 0.0f, 0.0f); // No texture, fully transparent

    float4 baseColor = gTexture.Sample(gSampler, input.Tex);

    if (MultiTexturing == 1)
    {
        float4 secondColor = gTextureSecondary.Sample(gSampler, input.Tex);
        baseColor.rgb = (baseColor.rgb * secondColor.rgb) * 2.0f;
        baseColor.a = max(baseColor.a, secondColor.a);
    }

    float3 N = normalize(input.Normal);
    float3 V = normalize(input.viewDirection);
    float3 worldPos = input.WorldPos;

    float3 albedo = baseColor.rgb;
    float3 finalRGB = float3(0.0f, 0.0f, 0.0f);

    // Skip lighting if all lights are disabled
    if (DirectionalLightCount + SpotLightCount + PointLightCount == 0)
    {
        return float4(0.0f, 0.0f, 0.0f, baseColor.a); // Pure black
    }

    for (int i = 0; i < DirectionalLightCount; ++i)
    {
        finalRGB += ComputeDirectionalLight(gDirectionalLights[i], N, V, albedo);
    }

    for (int i = 0; i < SpotLightCount; ++i)
    {
        finalRGB += ComputeSpotLight(gSpotLights[i], N, worldPos, V).rgb;
    }

    for (int i = 0; i < PointLightCount; ++i)
    {
        finalRGB += ComputePointLight(gPointLights[i], N, V, worldPos, albedo).rgb;
    }

    return float4(saturate(finalRGB), baseColor.a);
}
