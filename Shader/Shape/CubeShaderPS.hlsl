cbuffer LightMeta : register(b1)
{
    int gDirectionalLightCount;
    float3 padding;
};

struct DIRECTIONAL_Light_DATA
{
    float4 SpecularColor;
    float4 AmbientColor;
    float4 DiffuseColor;
    float3 Direction;
    float  SpecularPower;
};

// Structured buffer of directional lights (max 10)
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
    float4 textureColor = gTexture.Sample(gSampler, input.Tex);

    float4 finalColor = float4(0, 0, 0, 0);

    if (gDirectionalLightCount > 0)
    {
        finalColor = gDirectionalLights[0].AmbientColor * textureColor;
    }

    return saturate(finalColor);
}
