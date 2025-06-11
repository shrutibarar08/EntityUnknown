cbuffer WorldTransform : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix NormalMatrix;
};

cbuffer CameraBuffer : register(b1)
{
    float3 cameraPosition;
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
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);
    float4 viewPos  = mul(worldPos, ViewMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);
    output.Tex      = input.TexCoord;

    // Extract 3x3 matrix from NormalMatrix
    float3x3 normalMat = float3x3(
        NormalMatrix[0].xyz,
        NormalMatrix[1].xyz,
        NormalMatrix[2].xyz
    );
    output.Normal = normalize(mul(normalMat, input.Normal));

    output.viewDirection = normalize(cameraPosition - worldPos.xyz);

    return output;
}
