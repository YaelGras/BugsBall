RWTexture2D<float4> UAV : register(u0);

Texture2D worldPos : register(t0);
Texture2D sourceTex;

cbuffer cameraBuffer : register(b0)
{
    float4 camPos; // Example parameter
    float4 cameraRange;
    float4x4 viewProj;
    float4x4 view;
};

cbuffer tiltShiftParams : register(b1)
{
    uint sampleCount;
    float blurStrength;
    float radius;
};


float4 WorldToClipPos(float3 pos)
{
    return mul(viewProj, float4(pos, 1.f));
}

float3 WorldToViewPos(float3 pos)
{
    return mul(view, float4(pos, 1.0)).xyz;
}

float gaussian(float2 i)
{
    return exp(-.5 * dot(i /= (sampleCount * .25), i)) / sqrt(6.28 * (sampleCount * .25) * (sampleCount * .25));
}

float4 blur(Texture2D tex, float2 uv, float scale)
{
    float4 outBlur = float4(0, 0, 0, 0);
    
    for (int i = 0; i < sampleCount * sampleCount; i++)
    {
        float2 offset = float2(i % sampleCount, i / sampleCount) - sampleCount / 2.f;
        //O += gaussian(d) * sp.Load(int3(U + scale * d, 0));
        outBlur += gaussian(offset) * float4(tex.Load(int3(uv + offset * scale, 0)).rgb, 1.0f);
    }
    
    return outBlur / outBlur.a;
}



[numthreads(8, 4, 1)]
void CSMain(uint2 threadID : SV_DispatchThreadID)
{
    //UAV[threadID] = blur(sourceTex, threadID);
    uint w, h;
    UAV.GetDimensions(w, h);
    
    float4 wp = worldPos.Load(uint3(threadID, 0), 0);
    float2 uv = float2(threadID.x / (float) w, threadID.y / (float) h);
    float2 clipuv = uv * 2 - 1;
    
    float linDepth = WorldToViewPos(wp.xyz).z;
    float4 clipPos = WorldToClipPos(wp.xyz);
    float depth = clipPos.z / clipPos.w;    
    
    float2 center = float2(0.5, 0.5); // Center point of the effect
    
    float distanceFromCenter = distance(uv, center);
    
    // smoothing
    float blurFactor = smoothstep(radius - 0.15, radius + 0.15, distanceFromCenter);
    
    float4 blurred = blur(sourceTex, threadID, blurFactor * blurStrength * (1 + depth));
    float4 total = lerp(sourceTex[threadID], blurred, blurFactor);
    
    
    UAV[threadID] = total;
    
}

