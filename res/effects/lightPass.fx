//////////////////////

// GBUFFER

Texture2D albedo; // la texture
Texture2D normal; // la texture
Texture2D position; // la texture
Texture2D ambiantOcclusion; // la texture
Texture2D roughness; // la texture
Texture2D specular; // la texture
Texture2D unlitTexture; // la texture

Texture2D sunDepth ; // la texture

SamplerState SampleState
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
    Filter = MIN_MAG_MIP_LINEAR;
};

SamplerState shadowSampler
{
    AddressU = Wrap;
    AddressV = Wrap;
    Filter = MIN_MAG_MIP_POINT;
}; 


// -- Constant buffers 
cbuffer cameraParams
{
    float4x4 viewProj; // la matrice totale 
    float4 cameraPos; // la position de la caméra 
}

// -- Constant buffers 
cbuffer sunParams
{
    float4x4 sunViewProj; // la matrice totale 
    float4 sunRange;
    float4 actualsunPos;
}


float4 sunPos = float4(100.0f, 1000.f, 10.0f, 1.0f); // la position de la source d’éclairage (Point) 
float3 sunDir = normalize(float3(27.0f, 276.f, -508.0f)); // la position de la source d’éclairage (Point) 
float4 sunStrength = float4(0.45f, 0.45f, 0.45f, 0.45f); // la valeur diffuse de l’éclairage
float4 sunColor = float4(1.f, 1.f, .6f, 1.f); // la valeur ambiante de l’éclairage 

int LIGHT_COUNT = 16;
struct Light
{
    float4 direction; // For directional lights and spotlight
    float4 range; // for pl + radius of spotlights
    float4 position;
    float4 ambiant;
    float4 diffuse;
    
    float specularFactor; 
    float fallOff; // outside angle for spots
    float strength;
    float isOn;    
    
    uint type;
};

cbuffer lightsBuffer
{
    Light lights[16];
    
};


////////////////////////////////////////////////////////////////////////////////

// float2(0,1) - uv 

// -- VSOUT

struct VSOut
{
    float4 Pos : SV_Position;
    float2 uv : TEXCOORD0;
};


static const float4 vertices[6] =
{
    float4(-1, -1,-1, -1), // topleft
    float4(-1, +1,-1, +1), // bottomleft
    float4(+1, +1,+1, +1), // bottomright
    
    float4(-1, -1,-1, -1), // topleft
    float4(+1, +1,+1, +1), // bottomright
    float4(+1, -1, +1, -1), // topright
};

////////////////////////////////////////////////////////////////////////////////

// -- Vertex Shader

VSOut lightPassVS(uint id : SV_VertexID)
{
    VSOut vso;
    int actualId = int (id);
    vso.Pos = float4(vertices[actualId].xy, 0, 1);
    vso.uv = vertices[actualId].zw;
    
    return vso;
}

////////////////////
// Light computations




/////////////////////

float toonify(float diffuse)
{
    return smoothstep(-0.05f, +0.05f, diffuse - 0.5f) * diffuse;
}

float smoothfloor(float v)
{
    return floor(v) + smoothstep(0.8f, 1.F, frac(v));
}

float delinearize_depth(float d, float znear, float zfar)
{
    return znear * zfar / (zfar + d * (znear - zfar));
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * sunRange.x * sunRange.y) / (sunRange.y + sunRange.x - z * (sunRange.y - sunRange.x));
}

float isInShadow(float3 worldPos, float3 normal)
{
    
    
    float shadow = 0.0;
    uint w, h;
    sunDepth.GetDimensions(w, h);
    float2 texelSize = float2(1.0, 1.0) / float2(w, h);
    
    
    float4 lightSpace = mul(float4(worldPos, 1.0f), (sunViewProj));
    float2 uv = lightSpace.xy * .5f + .5f;
    
    uv = float2(uv.x, 1-uv.y);
    if (uv.x < 0 || uv.y < 0 || uv.x > 1 || uv.y > 1)
        return 0.f;
       
    float closestDepth = sunDepth.Sample(shadowSampler, uv).r;
    float actualDepth = lightSpace.z;
    
    float bias = max(0.1 * (1.0 - dot(normal, normalize(actualsunPos).xyz)), 0.1f) / (sunRange.y - sunRange.x);
   // return actualDepth - bias > closestDepth ? 1.0 : 0.0;
    
    int sampleCount = 4;
    for (int x = -sampleCount; x <= sampleCount; ++x)
    {
        for (int y = -sampleCount; y <= sampleCount; ++y)
        {
            float pcfDepth = sunDepth.Sample(shadowSampler, uv + float2(x, y) * texelSize).r;;
            shadow += actualDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= float(sampleCount * 2 + 1) * float(sampleCount * 2 + 1);
    if (lightSpace.z > 1.0)
        shadow = 0.0;   
    
    return shadow;
}

float distanceToSun(float3 worldPos)
{

    float2 uv = mul(sunViewProj, float4(worldPos, 1.0f)).xy * 0.5f + 0.5f;
    float depth = sunDepth.Sample(SampleState, uv).r;
    float delinear_depth = delinearize_depth(depth, sunRange.x, sunRange.y);
    return depth;
    
}


float3 computeSunDiffuseLight(float3 normal)
{
    
    return lerp(float3(0.19, 0.19, 0.19), sunColor.rgb, max(.3, toonify(saturate(dot(normal, sunDir))) * sunStrength.x));

}


// -- Fragment Shader

float4 lightPassPS(float4 sspos : SV_Position) : SV_Target
{
    float4 albedoValue  = albedo.Load(sspos.rgb);
    float4 fragnormal = normal.Load(sspos.rgb);
    float4 fragPos = position.Load(sspos.rgb);
    float specularPixel = specular.Load(sspos.rgb).x;
    float4 isUnlit = unlitTexture.Load(sspos.rgb);
    
    
    if (isUnlit.r != 0) return float4(isUnlit.rgb, 1.0f);
    
    float3 viewDir = normalize(fragPos.xyz - cameraPos.xyz);
    
    float diff = saturate(dot(normalize(sunDir), fragnormal.xyz));
    float isShadows = isInShadow(fragPos.xyz, fragnormal.xyz);
    
    
    float3 ambiant = ((sunColor.rgb) * 0.2f * sunStrength.r)  + albedoValue.rgb * 0.3f;
    float3 lighting = ambiant + (toonify(diff) * albedoValue.rgb);
    
    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
        if (lights[i].isOn < 1.0f) continue;
        
        if (lights[i].type == 1) // points
        {            
            float dist = length(lights[i].position.rgb - fragPos.rgb);
            float attenuation = 1.0 / (lights[i].range.y + lights[i].range.z * dist + lights[i].range.w * (dist * dist));
            float3 lightDir = normalize(lights[i].position.xyz - fragPos.xyz);
            float toonDiff = smoothfloor(3 * saturate(dot(lightDir, fragnormal.xyz))) / 3.f;
            toonDiff = saturate(dot(lightDir, fragnormal.xyz)); 
            
            float3 currlightDiff = toonDiff * albedoValue.rgb * lights[i].diffuse.rgb;
            float3 currLightAmbiant = lights[i].ambiant.rgb * albedoValue.rgb ;
            
            float3 H = normalize(lightDir + viewDir);
            float S = pow(max(dot(fragnormal.xyz, H), 0), 256.f);
            
            float3 currLightSpec = lights[i].diffuse.rgb * specularPixel * lights[i].specularFactor * diff;
         //   S = 1.0f;
            
            currlightDiff *= attenuation;    currLightAmbiant *= attenuation;    currLightSpec *= attenuation;
            
            lighting += currLightSpec;
            lighting += currLightAmbiant;
            lighting += currlightDiff;
            

        }
        if (lights[i].type == 2) // spots
        {

            // compute pixel angle
            float3 lightDir = normalize(lights[i].position.xyz - fragPos.xyz);
            float toonDiff = smoothfloor(3 * saturate(dot(lightDir, fragnormal.xyz))) / 3.f;
            toonDiff = saturate(dot(lightDir, fragnormal.xyz));
            
            float smallAngle = lights[i].range.x;
            float largeAngle = smallAngle + lights[i].fallOff;
            
            float pixelToSpotAngle = saturate(dot(
                    normalize(-lights[i].position.xyz + fragPos.xyz),
                    lights[i].direction.xyz
            ));
            
            
            float t = smoothstep(0, 1, (pixelToSpotAngle - cos(largeAngle)) / (cos(smallAngle) - cos(largeAngle)));
            
            float3 H = normalize(lightDir + viewDir);            
            float S = pow(max(dot(fragnormal.xyz, H), 0), 256.f);
            float3 currLightSpec = lights[i].diffuse.rgb * specularPixel * lights[i].specularFactor * diff * t;
            
            
            float3 currentAmbiant = t * lights[i].strength / 25.f;
            //lighting += currentAmbiant;
            //lighting += (t * lights[i].diffuse.rgb * lights[i].strength * albedoValue.rgb) * toonDiff;
            lighting += (t * lights[i].diffuse.rgb * lights[i].strength * albedoValue.rgb) * toonDiff + currLightSpec + lights[i].ambiant.rgb * t * lights[i].strength / 25.f;
            
        }
    }   
    float4 color = float4(lighting, 1.0f) + float4(toonify(specularPixel) * 10 * sunColor * 0.1);
  
    return color * lerp(1, .5f, isShadows);
   
   
    
    
}

////////////////////


// -- Technique

technique11 gPass
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, lightPassVS()));
        SetPixelShader(CompileShader(ps_5_0, lightPassPS()));
        SetGeometryShader(NULL);
    }
}