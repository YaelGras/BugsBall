
// Uniforms

TextureCube tex; // la texture
SamplerState SampleState
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
    Filter = MIN_MAG_MIP_LINEAR;
};

cbuffer SkyboxCbuffer
{
    matrix viewProj;
    float4 camPos;    
};

struct VSOut
{
    float3 worldPos : TEXCOORD0;
    float4 outpos : SV_Position;
    
};


VSOut skyboxVS(float3 pos: POSITION) 
{
    VSOut vso;
    vso.worldPos = pos;
    vso.outpos = mul(float4(pos+camPos.xyz, 1.0f), viewProj); // center at campos
    vso.outpos.z = vso.outpos.w; // z stays at 1 after w division
    return vso;
    
}

float4 skyboxFS(float3 worldPos : TEXCOORD0) : SV_TARGET
{
    return float4(tex.Sample(SampleState, worldPos).rgb, 1.F);
}

technique11 skybox
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, skyboxVS()));
        SetPixelShader(CompileShader(ps_5_0, skyboxFS()));
        SetGeometryShader(NULL);
    }
}