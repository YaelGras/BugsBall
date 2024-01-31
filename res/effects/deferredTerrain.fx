//////////////////////

// Uniforms

Texture2D ambiantOcclusion; // la texture
Texture2D albedo; // la texture
Texture2D normalMap; // la texture
Texture2D roughness; // la texture
Texture2D specular; // la texture

Texture2D grassTexture; // la texture
Texture2D rockTexture; // la texture
Texture2D snowTexture; // la texture

Texture2D snow_normal; // la texture
Texture2D grass_normal; // la texture
Texture2D rock_normal; // la texture

/////////////////////////////

SamplerState SampleState 
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
    Filter = MIN_MAG_MIP_LINEAR;
};


////////////////////

float3 sunDir = normalize(float3(27.0f, 276.f, -508.0f)); // la position de la source d’éclairage (Point) 
float4 sunStrength = float4(0.45f, 0.45f, 0.45f, 0.45f); // la valeur diffuse de l’éclairage
float4 sunColor = float4(1.f, 1.f, .6f, 1.f); // la valeur ambiante de l’éclairage 
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

struct VSOut
{
    float4 Pos : SV_Position;
    float3 Norm : TEXCOORD0;
    float4 worldPos : TEXCOORD1;
    float2 uv : TEXCOORD2;
};


struct PSOut
{
    float4 Normal : SV_Target0; //Normal map
    float4 Diffuse : SV_Target1; //Color
    float4 Position : SV_Target2;
    float4 Specular : SV_Target3;
    float4 ambiantOcclusion : SV_Target4;
    float4 roughness : SV_Target5;
};


////////////////////

// -- Vertex Shader

VSOut deferredTerrainVS(float4 Pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
    float4x4 MVP = mul(worldMat, viewProj);
    VSOut vs_out = (VSOut) 0;
    
        
    vs_out.Pos = mul(Pos, MVP);
    
    vs_out.Norm = mul(float4(normal, 0.0f), worldMat).xyz;
    vs_out.worldPos = mul(Pos, worldMat);
    vs_out.uv = uv;
    
    return vs_out;

   
}

////////////////////

// -- Fragment Shader
float toonify(float diffuse)
{
   // return step(0.5 - diffuse, diffuse-.5) ;
    return smoothstep(-0.05f, +0.05f, diffuse-0.5f) * diffuse;
}


PSOut deferredTerrainPS(VSOut vs) : SV_Target
{
    

    float3 outNormal = normalize(vs.Norm);
    float3 outAlbedo;
    
    float grassSnowLerp = saturate(vs.worldPos.y / 50.f);
    float baseRockLerp = smoothstep(0.30, 1, outNormal.y);
    
    // Compute the normal from the normal map

    float3 tempNormal;
    tempNormal= lerp(grass_normal.Sample(SampleState, vs.uv).rbg, snow_normal.Sample(SampleState, vs.uv).rbg, grassSnowLerp);
    tempNormal = lerp(rock_normal.Sample(SampleState, vs.uv).rbg, tempNormal, baseRockLerp);
    
    float3 randomVec = float3(1, 2, 3);
    float3 tan = normalize(cross(randomVec, outNormal));
    float3 bi = normalize(cross(tan, outNormal));
    
    tempNormal = normalize(tempNormal * 2.0 - 1.0);
    outNormal = tempNormal.x * tan + outNormal * tempNormal.y + tempNormal.z * bi;
    outNormal = normalize(outNormal);   
    

    // specular    
    
    float3 L = normalize(sunDir);
    float3 V = normalize(cameraPos - vs.worldPos).xyz;
    float3 H = normalize(L + V);
    float diff = saturate(dot(L, outNormal));
    float S = pow(max(dot(outNormal, H), 0), 512.f);
    //S = 0.f;
    
     // -- Compute albedo
    
    float3 rockSample = rockTexture.Sample(SampleState, vs.uv).rgb;
    float3 grassSample = grassTexture.Sample(SampleState, vs.uv).rgb;
    float3 snowSample = snowTexture.Sample(SampleState, vs.uv).rgb;

    float3 baseTexture = lerp(grassSample, snowSample, grassSnowLerp);
    outAlbedo.rgb = lerp(rockSample, baseTexture, baseRockLerp);
   
    // -- Output to RTV
    
    PSOut pso;
    pso.Normal = float4(outNormal, 1.0F);
    pso.Diffuse = float4(toonify(diff) * outAlbedo + outAlbedo * 0.3f, 1.0);
    pso.Diffuse = float4(outAlbedo, 1.0f);
    pso.Position = vs.worldPos;
    pso.Specular = float4(S, S, S, 1);
    pso.ambiantOcclusion = float4(ambiantOcclusion.Sample(SampleState, vs.uv).rgb, 1);
    pso.ambiantOcclusion = float4(diff * outAlbedo, 1);
    pso.roughness = float4(roughness.Sample(SampleState, vs.uv).rgb, 1);
    
    return pso;
    
}

////////////////////

// -- Technique

technique11 gPass
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, deferredTerrainVS()));
        SetPixelShader(CompileShader(ps_5_0, deferredTerrainPS()));
        SetGeometryShader(NULL);
    }
}