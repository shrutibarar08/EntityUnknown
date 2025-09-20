cbuffer WorldTransform : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix NormalMatrix;
    float3 CameraPosition;
    float _pad0;
};

struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 ViewDirection : TEXCOORD1;
    float3 WorldPos : TEXCOORD2;
    float3x3 TBN : TEXCOORD3;
};

VSOutput main(VSInput i)
{
    VSOutput o;

    float4 worldPos = mul(float4(i.Position, 1.0f), WorldMatrix);
    o.WorldPos = worldPos.xyz;

    float4 viewPos = mul(worldPos, ViewMatrix);
    o.Position = mul(viewPos, ProjectionMatrix);

    o.Tex = i.TexCoord;

    float3x3 Nmat = (float3x3) NormalMatrix;
    float3 N = normalize(mul(i.Normal, Nmat));
    float3 T = normalize(mul(i.Tangent, Nmat));
    T = normalize(T - N * dot(T, N));
    float3 B = normalize(cross(N, T));
    o.TBN = float3x3(T, B, N);

    o.ViewDirection = normalize(CameraPosition - o.WorldPos);
    return o;
}
