cbuffer WorldTransform : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    float3 cameraPosition;
    float padding; // for 16-byte alignment
};

struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VSOutput
{
    float4 Position       : SV_POSITION;
    float2 Tex            : TEXCOORD0;
    float3 ViewDirection  : TEXCOORD1;
    float3 WorldPos       : TEXCOORD2;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);
    float4 viewPos  = mul(worldPos, ViewMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);

    output.Tex = input.TexCoord;
    output.WorldPos = worldPos.xyz;

    // View direction from worldPos to camera
    output.ViewDirection = normalize(cameraPosition - worldPos.xyz);

    return output;
}
