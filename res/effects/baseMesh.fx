//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          DEPRECATED                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Uniforms

Texture2D tex; // la texture
SamplerState SampleState
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
    Filter = MIN_MAG_MIP_LINEAR;
};

////////////////////

// -- Constant buffers 
cbuffer worldParams
{
    float4x4 viewProj; // la matrice totale 
    float4 lightPos; // la position de la source d’éclairage (Point) 
    float4 cameraPos; // la position de la caméra 
    
    float4 sunColor; // la valeur ambiante de l’éclairage 
    float sunStrength; // la valeur diffuse de l’éclairage
}

cbuffer meshParams
{
    float4x4 worldMat; // matrice de transformation dans le monde
};

/////////////////////

// -- VSOUT

struct VSOut
{
    float4 Pos : SV_Position;
    float3 Norm : TEXCOORD0;
    float3 lightDir : TEXCOORD1;
    float3 camDir : TEXCOORD2;
    float2 uv : TEXCOORD3;
};

////////////////////

// -- Vertex Shader

VSOut baseMeshVS(float4 Pos : POSITION, float3 Normale : NORMAL, float2 uv : TEXCOORD)
{
    float4x4 MVP = mul(worldMat, viewProj);
    VSOut vs_out = (VSOut) 0;
    vs_out.Pos = mul(Pos, MVP);
    vs_out.Norm = mul(float4(Normale, 0.0f), worldMat).xyz;
    
    float3 worldPos = mul(Pos, worldMat).xyz;
    vs_out.lightDir = lightPos.xyz - worldPos;
    vs_out.camDir = cameraPos.xyz - worldPos;
    
    vs_out.uv = uv;
    return vs_out;
}

////////////////////

// -- Fragment Shader

float4 baseMeshPS(VSOut vs) : SV_Target
{
    float3 couleur;
    // Normaliser les paramètres 
    float3 N = normalize(vs.Norm);
    float3 L = normalize(vs.lightDir);
    float3 V = normalize(vs.camDir);
    
    // Valeur de la composante diffuse 
    float3 sunLight = saturate(dot(N, L));
    float3 R = normalize(2 * sunLight * N - L);
    // Puissance de 4 - pour l’exemple
    float S = pow(saturate(dot(R, L)), 4.f);

    //float3 texSample = tex.Sample(SampleState, vs.uv).rgb;
    
    couleur.rgb = float3(1,0,0);
    couleur.rgb *= lerp(float3(0.09, 0.09, 0.09), sunColor.rgb, max(.1, sunLight * sunStrength));
    couleur += S / 4.f; 

    return float4(couleur, 1.0f);
    
}

////////////////////

// -- Technique

technique11 MiniPhong
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, baseMeshVS()));
        SetPixelShader(CompileShader(ps_5_0, baseMeshPS()));
        SetGeometryShader(NULL);
    }
}