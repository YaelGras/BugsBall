//////////////////////

// Uniforms


////////////////////

// -- Constant buffers 
cbuffer cameraParams
{
    float4x4 viewProj; // la matrice totale 
    float4 cameraPos; // la position de la caméra     
}

cbuffer meshParams
{
    float4x4 worldMat; // matrice de transformation dans le monde
};


/////////////////////

// -- VSOUT

////////////////////

// -- Vertex Shader

float4 sunPassVS(float4 Pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD) : SV_Position
{
    float4x4 MVP = mul(worldMat, viewProj);
    float4 finalPos = mul(Pos, MVP);
    return finalPos;
}

////////////////////

// -- Technique

technique11 sunPass
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, sunPassVS()));
        SetPixelShader(NULL);
        SetGeometryShader(NULL);
    }
}