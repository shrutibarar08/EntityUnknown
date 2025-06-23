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
    float4 SpecularColor;         // 16 bytes
    float4 AmbientColor;          // 16 bytes
    float4 DiffuseColor;          // 16 bytes

    float3 Direction;             // 12 bytes
    float  SpecularPower;         // 4 bytes

    float4x4 ViewProjectMatrix;   // 64 bytes
};

struct SPOT_LIGHT_GPU_DATA
{
    float4 SpecularColor;         // 16 bytes
    float4 AmbientColor;          // 16 bytes
    float4 DiffuseColor;          // 16 bytes

    float3 Position;              // 12 bytes
    float  Range;                 // 4 bytes

    float3 Direction;            // 12 bytes
    float  SpotAngleCosine;      // 4 bytes

    float  SpecularPower;        // 4 bytes
    float3 Padding;              // 12 bytes

    float4x4 ViewProjectMatrix;   // 64 bytes
};

struct POINT_LIGHT_GPU_DATA
{
    float4 SpecularColor;         // 16 bytes
    float4 AmbientColor;          // 16 bytes
    float4 DiffuseColor;          // 16 bytes

    float3 Position;              // 12 bytes
    float  Range;                 // 4 bytes

    float  SpecularPower;         // 4 bytes
    float3 Padding;               // 12 bytes

    float4x4 ViewProjectMatrix;   // 64 bytes
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

Texture2DArray<float> gDirectionalShadowMaps : register(t15);
Texture2DArray<float> gPointLightShadowMaps : register(t16); // skipping this for now hehehe
Texture2DArray<float> gSpotLightShadowMaps : register(t17);

SamplerState gSampler          : register(s0);
SamplerComparisonState gShadowSampler : register(s1);

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Tex      : TEXCOORD0;
    float3 viewDirection  : TEXCOORD1;
    float3 WorldPos       : TEXCOORD2;
};

float4 main(VSOutput input) : SV_TARGET
{
    // If bDebugLine is enabled, render solid green
    if (bDebugLine == 1)
    {
        return float4(0.0f, 1.0f, 0.0f, 1.0f); // Pure green with full alpha
    }
    // Sample base texture color
    float4 baseColor = gTexture.Sample(gSampler, input.Tex);
    return baseColor;
}
