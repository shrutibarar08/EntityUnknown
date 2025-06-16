cbuffer LightMeta : register(b0)
{
    int DirectionalLightCount;
    int SpotLightCount;
    int PointLightCount;
    int DebugLine;
    int MultiTexturing; // 0 means no, 1 means yes
    float3 padding;
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

Texture2D gTexture : register(t1);
Texture2D gTexture2nd  : register(t4); // Optional 2nd
SamplerState gSampler : register(s0);

StructuredBuffer<DIRECTIONAL_LIGHT_GPU_DATA> gDirectionalLights : register(t0);
StructuredBuffer<SPOT_LIGHT_GPU_DATA> gSpotLights : register(t2);
StructuredBuffer<POINT_LIGHT_GPU_DATA> gPointLights: register(t3);

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

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Tex      : TEXCOORD0;
    float3 viewDirection  : TEXCOORD1;
    float3 WorldPos       : TEXCOORD2;
};

float4 main(VSOutput input) : SV_TARGET
{
    // If DebugLine is enabled, render solid green
    if (DebugLine == 1)
    {
        return float4(0.0f, 1.0f, 0.0f, 1.0f); // Pure green with full alpha
    }

    // Sample base texture color
    float4 baseColor = gTexture.Sample(gSampler, input.Tex);

    // Apply multi-texturing blend if enabled
    if (MultiTexturing == 1)
    {
        float4 secondColor = gTexture2nd.Sample(gSampler, input.Tex);

        // Blend equation: (base * second) * 2.0
        baseColor.rgb = (baseColor.rgb * secondColor.rgb) * 2.0f;
        baseColor.a   = max(baseColor.a, secondColor.a); // Conservative alpha blend
    }

    // 2D surface assumed to face -Z
    float3 N = float3(0.0f, 0.0f, -1.0f);
    float3 V = normalize(input.viewDirection);
    float3 worldPos = input.WorldPos;

    // Base ambient contribution
    float3 finalColor = baseColor.rgb * 0.1f;

    // === Directional Lights ===
    for (int i = 0; i < DirectionalLightCount; ++i)
    {
        finalColor += ComputeDirectionalLight(gDirectionalLights[i], N, V, baseColor.rgb);
    }

    // === Spot Lights ===
    for (int i = 0; i < SpotLightCount; ++i)
    {
        finalColor += ComputeSpotLight(gSpotLights[i], N, worldPos, V).rgb;
    }

    // === Point Lights ===
    for (int i = 0; i < PointLightCount; ++i)
    {
        finalColor += ComputePointLight(gPointLights[i], N, V, worldPos, baseColor.rgb);
    }

    return float4(saturate(finalColor), baseColor.a);
}
