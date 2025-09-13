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
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

struct VSOutput
{
    float4 Position      : SV_POSITION;
    float2 Tex           : TEXCOORD0;
    float3 ViewDirection : TEXCOORD1;
    float3 WorldPos      : TEXCOORD2;
    float3x3 TBN         : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    // === Transform Position to World Space ===
    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);
    output.WorldPos = worldPos.xyz;

    // === Project to Clip Space ===
    float4 viewPos = mul(worldPos, ViewMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);

    // === Pass UV ===
    output.Tex = input.TexCoord;

    // === Normal Matrix ===
    float3x3 normalMat = (float3x3)NormalMatrix;

    // === Transform TBN vectors to world space ===
    float3 worldNormal   = normalize(mul(input.Normal,   normalMat));
    float3 worldTangent  = normalize(mul(input.Tangent,  normalMat));
    float3 worldBinormal = normalize(mul(input.Binormal, normalMat));

    // === Store TBN Matrix ===
    output.TBN = float3x3(worldTangent, worldBinormal, worldNormal);

    // === View Direction in world space ===
    output.ViewDirection = normalize(CameraPosition - output.WorldPos);

    return output;
}
