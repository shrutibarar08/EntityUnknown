Texture2D gTexture : register(t1);
SamplerState gSampler : register(s0);

struct VSOutput
{
    float4 Position       : SV_POSITION;
    float2 Tex            : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET
{
    return gTexture.Sample(gSampler, input.Tex);
}
