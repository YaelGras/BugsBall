//////////////////////

// Uniforms

Texture2D origin; // la texture
Texture2D end; // la texture
float4 u_time;

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
    float4(-1, -1, 0, 0),
    float4(+1, +1, 1, 1),
    float4(+1, -1, 1, 0),
    float4(-1, +1, 0, 1),
    float4(+1, +1, 1, 1),
    float4(-1, -1, 0, 0)
};


float4 blackSweepTransition(float2 uv, float4 o, float4 d, float time, float duration)
{
    float actualTime = time * 2;
    float4 pixelColor = float4(0, 0, 0, 1);
       
    pixelColor += smoothstep(-0.05f, +0.05f, uv.x - actualTime) * o;
    pixelColor += smoothstep(0.05f, -0.05f, uv.x - (actualTime - duration)) * d;
    

    return pixelColor;

}


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
    
    float4 originSample = origin.Sample(SampleState, vs.uv);
    float4 destSample = end.Sample(SampleState, vs.uv);
    return blackSweepTransition(vs.uv, originSample, destSample, u_time.x, u_time.y);
    
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