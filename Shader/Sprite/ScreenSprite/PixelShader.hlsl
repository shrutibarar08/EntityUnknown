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
    return baseColor;
}
