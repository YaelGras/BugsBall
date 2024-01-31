RWTexture2D<float4> UAV : register(u0);
SamplerState ss;
Texture2D sourceTex : register(t0);

cbuffer cameraBuffer : register(b0)
{
    float4 camPos; // Example parameter
    float4 cameraRange;
    float4x4 viewProj;
    float4x4 view;
};
//////////////////////////////////////////////

cbuffer radialBlurParams
{
    float4 intensity;
    float4 sampleCount;
    float4 radius;
    float4 centerPoint;
};

//////////////////////////////////////////////

[numthreads(8, 8, 1)]
void CSMain(uint2 threadID : SV_DispatchThreadID)
{
    
    int width, height;
    
    sourceTex.GetDimensions(width, height);
    float2 srcResolution = float2(width, height);
    float2 uv = float2(threadID.x / (float) width, threadID.y / (float) height);
    
    // Blur
    float4 col = float4(0, 0, 0, 0);
    float2 dist = uv - centerPoint.xy;
    
    for (int j = 0; j < sampleCount.r; j++)
    {
        float scale = 1 - intensity.r * (j / sampleCount.r) * (saturate(length(dist) / radius.r));
        col += sourceTex.SampleLevel(ss, dist * scale + centerPoint.xy, 0);
    }
    col /= sampleCount.r;
    
    UAV[threadID] = col;
}