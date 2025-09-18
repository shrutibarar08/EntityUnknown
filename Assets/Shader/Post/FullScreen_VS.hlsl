struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOut main(uint id : SV_VertexID)
{
    float2 p = float2((id == 2) ? 3.0 : -1.0,
                      (id == 1) ? 3.0 : -1.0);
    VSOut o;
    o.pos = float4(p, 0.0, 1.0);
    o.uv = p * float2(0.5, -0.5) + float2(0.5, 0.5);
    return o;
}
