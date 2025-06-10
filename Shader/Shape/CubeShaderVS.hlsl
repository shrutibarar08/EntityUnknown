cbuffer WorldTransform : register(b0)
{
    matrix TransformationMatrix;  // world
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

struct VSInput
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float4 Color        : COLOR;
    float2 TexCoord     : TEXCOORD;
};

struct VSOutput
{
    float4 Position     : SV_POSITION;
    float4 Color        : COLOR;
    float3 WorldPos     : TEXCOORD0;
    float3 Normal       : TEXCOORD1;
    float2 TexCoord     : TEXCOORD2;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    // Convert input position to world space
    float4 worldPos = mul(float4(input.Position, 1.0f), TransformationMatrix);
    output.WorldPos = worldPos.xyz;

    // Full transformation to clip space
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);

    // Pass color and texcoords through
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;

    // Transform normal (assumes uniform scale)
    output.Normal = mul((float3x3)TransformationMatrix, input.Normal);

    return output;
}
