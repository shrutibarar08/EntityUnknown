Texture2D Src : register(t0);
SamplerState Samp : register(s0);

cbuffer POSTFX_COMMON_PS_CB : register(b0)
{
    float iTime;
    float iTimerDelta;
    float iTimeFrameRate;
    int iFrame;

    float3 iResolution;
    float _padRes;

    float4 iMouse;

    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;

    float3 CameraPosition;
    float _padCam;

    float4 ExtraPram_1;
    float4 ExtraPram_2;
    float4 ExtraPram_3;
};

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 main(VSOut i) : SV_Target
{
    float3 color = Src.Sample(Samp, i.uv).rgb;
    return float4(color, 1.0);
}
