//////////////////////

// Uniforms

Texture2D tex; // la texture
float4 colorShift;

SamplerState SampleState
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
    Filter = MIN_MAG_MIP_LINEAR;
};


// -- VSOUT

struct VSOut
{
    float4 Pos : SV_Position;
    float2 uv : TEXCOORD0;
};

static const float4 vertices[6] = 
{
    float4(-1, -1   , 0, 0),
    float4(+1, +1   , 1, 1),
    float4(+1, -1   , 1, 0),
    float4(-1, +1   , 0, 1),
    float4(+1, +1   , 1, 1),
    float4(-1, -1   , 0, 0)
};

////////////////////

// -- Vertex Shader

VSOut blitVS(uint id : SV_VertexID)
{
    
    VSOut vso;
    vso.Pos = float4(vertices[id].xy, 0, 1);
    vso.uv = vertices[id].zw;
    vso.uv.y = 1 - vso.uv.y;
    
    return vso;
}

////////////////////

// -- Fragment Shader

float4 blitPS(VSOut vs) : SV_Target
{
    
    float4 sample = tex.Sample(SampleState, vs.uv);
    return float4(sample.rgb * colorShift.rgb, colorShift.a);
    
}

////////////////////

// -- Technique

technique11 blit
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, blitVS()));
        SetPixelShader(CompileShader(ps_5_0, blitPS()));
        SetGeometryShader(NULL);
    }
}