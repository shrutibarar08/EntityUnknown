Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float3 WorldPos : TEXCOORD0;
    float3 Normal   : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
};

float4 main(VSOutput input) : SV_TARGET
{
    float4 textureColor = gTexture.Sample(gSampler, input.TexCoord);
    return textureColor;
}
