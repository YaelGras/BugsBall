
Texture2D tex;
SamplerState SampleState
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
    Filter = MIN_MAG_MIP_LINEAR;
};

struct VertexInput
{
    float3 pos          : POSITION;
    float2 uv           : TEXCOORD0;
    
    float3 instancePos  : INSTANCE_POS;
    float2 instanceSize : INSTANCE_SIZE;
    float4 instanceUV   : INSTANCE_UV;    
    float4 instanceCOLOR   : INSTANCE_COL;    
};

struct VertexOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 col : TEXCOORD1;
};

VertexOut sprite2DVS(VertexInput VSin) 
{
    VertexOut vso;
    vso.pos = float4((VSin.pos.xy * VSin.instanceSize + VSin.instancePos.xy ),0, 1);
    vso.uv = VSin.uv * VSin.instanceUV.zw + VSin.instanceUV.xy;
    //vso.uv = float2(vso.uv.x, 1 - vso.uv.y);
    vso.col = VSin.instanceCOLOR;
    return vso;
}

float4 sprite2DPS(VertexOut vsin) : SV_Target
{    
   return float4(tex.Sample(SampleState, vsin.uv)) * vsin.col;
    //return float4(1,0,0, 1);
}


technique11 sprite2D
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, sprite2DVS()));
        SetPixelShader(CompileShader(ps_5_0, sprite2DPS()));
        SetGeometryShader(NULL);
    }
}