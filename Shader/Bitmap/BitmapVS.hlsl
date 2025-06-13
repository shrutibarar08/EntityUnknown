cbuffer WorldTransform : register(b0)
{
    matrix WorldMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    float3 cameraPosition;
	float padding;
};


struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 Tex      : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(float4(input.Position, 1.0f), WorldMatrix);
    float4 viewPos  = mul(worldPos, ViewMatrix);
    output.Position = mul(viewPos, ProjectionMatrix);

    output.Tex = input.TexCoord;
    return output;
}
