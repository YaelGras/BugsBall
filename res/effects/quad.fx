//////////////////////

// Uniforms

Texture2D whitePixel; // la texture
Texture2D tex_slots[8];

SamplerState SampleState
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
    Filter = MIN_MAG_MIP_LINEAR;
};

struct VSIn
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
};

// -- VSOUT

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    int texId : TEXCOORD1;
};


////////////////////

// -- Vertex Shader

VSOut quadVS(VSIn vsin)
{
    
    VSOut vso;
    vso.pos     = float4(vsin.pos.xy, 0, 1);
    vso.uv      = float2(vsin.uv.x,1- vsin.uv.y);
    vso.texId = int(vsin.pos.z);
    return vso;
}

////////////////////

// -- Fragment Shader

float4 quadPS(VSOut vs) : SV_Target
{
    float4 texSample = (0,0,0,1);
    
    for (int i = 0; i < 8; i++)
    {
        if (i==vs.texId)
        {
            texSample = tex_slots[i].Sample(SampleState, vs.uv);            
        }

    }    
    return texSample;
    
}
////////////////////

// -- Technique

technique11 quad
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, quadVS()));
        SetPixelShader(CompileShader(ps_5_0, quadPS()));
        SetGeometryShader(NULL);
    }
}