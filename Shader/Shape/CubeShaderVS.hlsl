cbuffer WorldTransform : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix NormalMatrix;
    float3 CameraPosition;
    float padding; // 16-byte alignment
};

struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal   : NORMAL;
};

struct VSOutput
{
    float4 Position       : SV_POSITION;
    float2 Tex            : TEXCOORD0;
    float3 Normal         : TEXCOORD1;
    float3 viewDirection  : TEXCOORD2;
    float3 WorldPos       : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    // === Transform Position to World Space ===
    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);
    output.WorldPos = worldPos.xyz;

    // === View/Projection ===
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);

    // === TexCoord Pass-through ===
    output.Tex = input.TexCoord;

    // === Normal Transformation ===
    float3x3 normalMat = (float3x3)NormalMatrix;
    output.Normal = normalize(mul(input.Normal, normalMat));

    // === View Direction (world space) ===
    output.viewDirection = normalize(CameraPosition - worldPos.xyz);

    return output;
}
