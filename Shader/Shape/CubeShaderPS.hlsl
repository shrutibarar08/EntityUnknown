cbuffer LightMeta : register(b0)
{
    int gDirectionalLightCount;
    int DebugLine;
    float2 padding; // To align to 16 bytes
};

struct DIRECTIONAL_Light_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;
    float3 Direction;
    float  SpecularPower;
};

StructuredBuffer<DIRECTIONAL_Light_DATA> gDirectionalLights : register(t0);
Texture2D gTexture : register(t1);
SamplerState gSampler : register(s0);

struct VSOutput
{
    float4 Position       : SV_POSITION;
    float2 Tex            : TEXCOORD0;
    float3 Normal         : TEXCOORD1;
    float3 viewDirection  : TEXCOORD2;
};

float4 main(VSOutput input) : SV_TARGET
{
    // If DebugLine is enabled, render solid green
    if (DebugLine == 1)
    {
        return float4(0.0f, 1.0f, 0.0f, 1.0f); // Pure green with full alpha
    }

    float4 baseColor = gTexture.Sample(gSampler, input.Tex);
    float3 N = normalize(input.Normal);
    float3 V = normalize(input.viewDirection);

    float4 finalColor = float4(0, 0, 0, 1);

    finalColor.rgb += baseColor.rgb * float3(0.2, 0.2, 0.25); // Global ambient

    for (int i = 0; i < gDirectionalLightCount; ++i)
    {
        DIRECTIONAL_Light_DATA light = gDirectionalLights[i];
        float3 L = normalize(-light.Direction);
        float3 H = normalize(L + V);

        float NdotL = max(dot(N, L), 0.0f);
        float NdotH = max(dot(N, H), 0.0f);

        float specIntensity = pow(NdotH, max(light.SpecularPower, 1.0f));

        float3 diffuse = light.DiffuseColor.rgb * baseColor.rgb * NdotL;
        float3 specular = light.SpecularColor.rgb * specIntensity;

        finalColor.rgb += diffuse + specular;
    }

    return saturate(finalColor);
}
