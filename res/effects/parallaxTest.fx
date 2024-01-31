//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          DEPRECATED                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Uniforms

Texture2D tex; // la texture
Texture2D heightmap;
Texture2D ao;
Texture2D normalMap;
Texture2D specular;
SamplerState SampleState
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
    Filter = MIN_MAG_MIP_LINEAR;
};

float heightScale = 0.1f;
float minLayer = 4.0f;
float maxLayer = 128.0f;
float3 sunPos = float3(100, 100, 100);

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
    float4 tanPos : TEXCOORD4;
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
    float4 N = float4(normalize(Normale),0.0f);
    float4 randomVector = float4(normalize(float3(1, 2, 3)), 0.0F);
    float4 tan = float4(normalize(cross(randomVector, N)),  0.0);
    float4 bi = float4(normalize(cross(tan, N)), 0.0f);
    float4x4 tbn = transpose(float4x4(tan, bi, N, float4(0,0,0,1) ));
    
    vs_out.tanPos = mul(float4(Pos.xyz, 1.0f), tbn);
    
    return vs_out;
}

////////////////////

// -- Fragment Shader

float4 baseMeshPS(VSOut vs) : SV_Target
{
    
    
    
    
    float3 N = normalize(vs.Norm);
    float3 randomVector = normalize(float3(1, 2, 3));
    float3 tan = normalize(cross(randomVector, N));
    float3 bi = normalize(cross(tan, N));    
    float3x3 tbn = float3x3(tan, bi, N);
    
    float3 viewDir = normalize(vs.tanPos.xyz - (mul(tbn, cameraPos.xyz))).xyz;
    
    // Heightscales

    
    float numLayer = lerp(maxLayer, minLayer, abs(dot(float3(0, 0, 1), viewDir)));
    float layerDepth = 1.0F / numLayer;
    float currentLayerDepth = 0.0F;
    
    float2 S = viewDir.xy / viewDir.z * heightScale;
    float2 duvs = S / numLayer;
    float2 uv = vs.uv;
    float currentDepthMapValue = 1.0f - heightmap.Sample(SampleState, uv).r;
    
    [loop] while(currentLayerDepth < currentDepthMapValue)
    {
        uv -= duvs;
        currentDepthMapValue = 1.0f - heightmap.Sample(SampleState, uv).r;
        currentLayerDepth += layerDepth;
    }
    
    float2 prevTexCoords = uv + duvs;
    float afterDepth = currentDepthMapValue + currentLayerDepth;
    float beforeDepth = 1.0f - heightmap.Sample(SampleState, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    uv = prevTexCoords * weight + uv * (1.0f - weight);
    
    if (uv.x > 1.0F || uv.x < 0.0f || uv.y > 1.0F || uv.y < 0.0F)
    {
        discard;
    }
    
        
    float3 tempNormal = normalize(normalMap.Sample(SampleState, uv) * 2.0 - 1.0);
    N = tempNormal.x * tan + N * tempNormal.y + tempNormal.z * bi;
    N = normalize(N);
    
    
    float4 ambiant = tex.Sample(SampleState, uv) * pow(ao.Sample(SampleState, uv), 12.f) ;
    
    
    float3 lightDir = normalize(sunPos - vs.tanPos.xyz);
    float diff = saturate(dot(lightDir, N));
    float3 diffuse = diff * float3(0.74,0.48,0.26) * 0.25;
    
    float shininess = specular.Sample(SampleState, uv).r;
    float3 reflectDir = reflect(-lightDir, N);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(N, halfwayDir), 0.0), 4.0f);
    
    
    return float4(ambiant.rgb + diffuse +  float3(spec, spec, spec), 1.0F);
    return float4(N, 1.0F);
    return ambiant;
    return float4(float3(spec, spec, spec), 1.0F);
    
    
    
    
    
    
    
    
    
    return float4(tan, 1.0F);
    
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