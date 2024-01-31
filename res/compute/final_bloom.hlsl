RWTexture2D<float4> UAV : register(u0);
Texture2D lastMip; // we need srv for sampling

cbuffer cameraBuffer : register(b0)
{
    float4 camPos; // Example parameter
    float4 cameraRange;
    float4x4 viewProj;
    float4x4 view;
};
//////////////////////////////////////////////

SamplerState ss;


cbuffer bloomBuffer
{
    float4 Strength;
};

float3 aces(float3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}


[numthreads(32, 32, 1)]
void CSMain(uint2 threadID : SV_DispatchThreadID, uint2 groupId : SV_GroupThreadID)
{
    
    float3 result = float3(0,0,0);
    
    int width, height;
    UAV.GetDimensions(width, height);
    float2 srcResolution = float2(width, height);
    float2 uv = float2(threadID.x / (float) width, threadID.y / (float) height);
    
    
    float3 hdrColor = UAV.Load(uint3(threadID, 0)).rgb;
    float3 bloomColor = lastMip.SampleLevel(ss, uv, 0).rgb;
    result = lerp(hdrColor, bloomColor, Strength.x); // linear interpolation
    // tone mapping
    
    result = float3(1.0, 1.0, 1.0) - exp(-result * Strength.y);


    UAV[threadID] = float4(aces(result), 1.0);

}

