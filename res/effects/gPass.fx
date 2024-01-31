//////////////////////

// Material of the mesh

Texture2D ambiantOcclusion; 
Texture2D albedo; 
Texture2D normalMap;
Texture2D roughness; 
Texture2D specular; 

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
    float4 Albedo : SV_Target1; //Color
    float4 Position : SV_Target2;
    float4 Specular : SV_Target3;
    float4 ambiantOcclusion : SV_Target4;
    float4 roughness : SV_Target5;
};


////////////////////

// -- Vertex Shader

VSOut gPassVS(float4 Pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
    float4x4 MVP = mul(worldMat, viewProj);
    VSOut vs_out = (VSOut) 0;
    vs_out.Pos = mul(Pos, MVP);
    vs_out.Norm = mul(float4(normal, 0.0f), worldMat).xyz;
    vs_out.worldPos = mul(Pos, worldMat);
    vs_out.worldPos.xyz /= vs_out.worldPos.w;
    vs_out.uv = float2(uv.x, 1 - uv.y);
    
    return vs_out;
}

////////////////////

// -- Fragment Shader


float toonify(float diffuse)
{
    return smoothstep(-0.05f, +0.05f, diffuse - 0.5f) * diffuse;
}


PSOut gPassPS(VSOut vs) : SV_Target
{
    float3 outNormal = normalize(vs.Norm);// * normalMap.Sample(SampleState, vs.uv);
    
    float3 prout = normalize(float3(1, 2, 3));
    float3 tan = normalize(cross(prout, outNormal));
    float3 bi = normalize(cross(tan, outNormal));
    
    
    float4 tempNormal = normalMap.Sample(SampleState, vs.uv);
    
    if (tempNormal.a != 0)
    {        
        tempNormal = normalize(tempNormal * 2.0 - 1.0);
        outNormal = tempNormal.x * tan + vs.Norm * tempNormal.y + tempNormal.z * bi;
        outNormal = normalize(outNormal);
    }
    
    float3 texSample = albedo.Sample(SampleState, vs.uv).rgb;
    float4 specularSample = specular.Sample(SampleState, vs.uv);    
    float3 outAlbedo;
    outAlbedo.rgb = texSample;   

    // sun specular    
    
    float3 L = normalize(sunDir);
    float3 V = normalize(cameraPos - vs.worldPos);   
    float3 H = normalize(L + V);    
    float diff = saturate(dot(L, outNormal));
    float S = pow(max(dot(outNormal, H),0), 128.f);

    //outAlbedo.rgb *= lerp(float3(0.09, 0.09, 0.09), sunColor.rgb, max(.1, toonify(diff) * sunStrength.x));
    
    //outAlbedo.rgb = toonify(diff) * outAlbedo + outAlbedo * 0.05f;
    
    
    //////////////////////////////////////
    /// Outline
    float dotProduct = dot(normalize(vs.Norm), V);
    dotProduct = step(0.3f, dotProduct);
    //outAlbedo.rgb *= dotProduct;
    outAlbedo.rgb = outAlbedo.rgb + (sunColor.rgb * 2.f.rrr + 1.f.rrr) * smoothstep(0.03f, -0.03f, dotProduct);
        
    //////////////////////////////////////
    
    PSOut pso;
    pso.Normal = float4(outNormal, 1.0F);
    pso.Albedo = float4(outAlbedo, 1.0f);
    //pso.Albedo = float4(albedo.Sample(SampleState, vs.uv).rgb, 1.0f);
    pso.Position = vs.worldPos;
    pso.Specular = float4(specularSample) + float4(S, S, S, 1 - specularSample.a);
    pso.ambiantOcclusion = float4(ambiantOcclusion.Sample(SampleState, vs.uv).rgb ,1);
    pso.ambiantOcclusion = float4(toonify(diff) * outAlbedo, 1);
    pso.roughness = float4(vs.uv, 0 ,1);
    
    return pso;
    
}

////////////////////

// -- Technique

technique11 gPass
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, gPassVS()));
        SetPixelShader(CompileShader(ps_5_0, gPassPS()));
        SetGeometryShader(NULL);
    }
}