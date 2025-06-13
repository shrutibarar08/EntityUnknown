cbuffer LightMeta : register(b0)
{
    int gDirectionalLightCount;
    float3 padding; // To align to 16 bytes
};

struct DIRECTIONAL_Light_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;
    float3 Direction;
    float  SpecularPower;
};

// Structured buffer of directional lights (max 10, or dynamic)
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
    float4 baseColor = gTexture.Sample(gSampler, input.Tex);
    float3 N = normalize(input.Normal);
    float3 V = normalize(input.viewDirection);

    float4 finalColor = float4(0, 0, 0, 1); // Start with alpha = 1

    for (int i = 0; i < gDirectionalLightCount; ++i)
    {
        DIRECTIONAL_Light_DATA light = gDirectionalLights[i];
        float3 L = normalize(-light.Direction); // Light direction points *toward* surface
        float3 H = normalize(L + V);            // Half-vector for Blinn-Phong

        // Ambient
        float4 ambient = light.AmbientColor * baseColor;

        // Diffuse (Lambert)
        float NdotL = max(dot(N, L), 0.0f);
        float4 diffuse = light.DiffuseColor * baseColor * NdotL;

        // Specular (Blinn-Phong)
        float NdotH = max(dot(N, H), 0.0f);
        float specPower = max(light.SpecularPower, 1.0f); // Avoid pow(0, 0)
        float specIntensity = pow(NdotH, specPower);
        float4 specular = light.SpecularColor * specIntensity;

        finalColor.rgb += ambient.rgb + diffuse.rgb + specular.rgb;
    }

    return saturate(finalColor);
}
