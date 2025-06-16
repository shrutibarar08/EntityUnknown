cbuffer LightMeta : register(b0)
{
    int gDirectionalLightCount;
    float3 padding; // To align to 16 bytes
};

struct DIRECTIONAL_LIGHT_GPU_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;
    float3 Direction;
    float  SpecularPower;
};

StructuredBuffer<DIRECTIONAL_LIGHT_GPU_DATA> gDirectionalLights : register(t0);

Texture2D gTexture : register(t1);
SamplerState gSampler : register(s0);

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Tex      : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET
{
    float4 texColor = gTexture.Sample(gSampler, input.Tex);
    if (gDirectionalLightCount <= 0) return texColor;

    float3 normal = float3(0.0f, 0.0f, -1.0f);

    float4 totalAmbient = float4(0, 0, 0, 1);
    float4 totalDiffuse = float4(0, 0, 0, 0);

    float toonStep = 0.25f; // 4-band toon

    for (int i = 0; i < gDirectionalLightCount; ++i)
    {
        DIRECTIONAL_LIGHT_GPU_DATA light = gDirectionalLights[i];

        float3 lightDir = normalize(-light.Direction);
        float NdotL = max(dot(normal, lightDir), 0.0f);

        // Quantize lighting for toon effect
        NdotL = floor(NdotL / toonStep) * toonStep;

        totalAmbient += light.AmbientColor;
        totalDiffuse += light.DiffuseColor * NdotL;
    }

    float4 lighting = saturate(totalAmbient + totalDiffuse);
    return texColor * lighting;
}
