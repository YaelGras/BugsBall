//////////////////////

// -- Constant buffers 
cbuffer worldParams
{
    float4x4 viewProj;
}

/////////////////////

// -- VSOUT

struct VSOut
{
    float4 Pos : SV_Position;
};

////////////////////

// -- Vertex Shader

VSOut debugLineVs(float4 pos : POSITION)
{
    VSOut vso;
    vso.Pos = mul(pos, transpose(viewProj));
    return vso;
}

////////////////////



float4 debugLinePs(VSOut vs) : SV_Target
{
    return float4(1.f, 1.f, 0.f, 1.f);
}

////////////////////

// -- Technique

technique11 debugline
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, debugLineVs()));
        SetPixelShader(CompileShader(ps_5_0, debugLinePs()));
        SetGeometryShader(NULL);
    }
}