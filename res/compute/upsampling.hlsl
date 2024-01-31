RWTexture2D<float4> UAV : register(u0);
Texture2D source; // we need srv for sampling

////////////////////////////////////////////// UNUSED
cbuffer cameraBuffer : register(b0)
{
    float4 camPos; // Example parameter
    float4 cameraRange;
    float4x4 viewProj;
    float4x4 view;
};
//////////////////////////////////////////////

cbuffer bloomBuffer
{
    float4 Radius;
};


SamplerState ss;



[numthreads(8, 8, 1)]
void CSMain(uint2 threadID : SV_DispatchThreadID, uint2 groupId : SV_GroupThreadID)
{
    
    // Find the correct sample step disatnce
    int width, height;
    UAV.GetDimensions(width, height);
    
    float2 srcResolution = float2(width, height);
    float2 uv = float2(threadID.x / (float) (width - 1), threadID.y / (float) (height - 1));
    
    float2 srcTexelSize = 1.0 / srcResolution;

    float x = Radius; //Radius;
    float y = Radius; //Radius;
    
    
    uint2 texelCoord = uint2(threadID.xy);
    
    
    float u = float(texelCoord.x) / (groupId.x * 32);
    float v = float(texelCoord.y) / (groupId.y * 32);
    uv = float2(uv);
    
    //float u = float(texelCoord.x) / (64 * 32);
    //float v = float(texelCoord.y) / (32 * 32);
    //uv = (u, v);
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    
        
    float3 a = source.SampleLevel(ss, float2(uv.x - x, uv.y + y     ), 0).rgb;
    float3 b = source.SampleLevel(ss, float2(uv.x, uv.y +  y        ), 0).rgb;
    float3 c = source.SampleLevel(ss, float2(uv.x + x, uv.y + y     ), 0).rgb;
    
    float3 d = source.SampleLevel(ss, float2(uv.x -  x  , uv.y        ), 0).rgb;
    float3 e = source.SampleLevel(ss, float2(uv.x, uv.y), 0).rgb;
    float3 f = source.SampleLevel(ss, float2(uv.x +  x  , uv.y        ), 0).rgb;
    
    float3 g = source.SampleLevel(ss, float2(uv.x -  x  , uv.y -  y   ), 0).rgb;
    float3 h = source.SampleLevel(ss, float2(uv.x       , uv.y -  y   ), 0).rgb;
    float3 i = source.SampleLevel(ss, float2(uv.x +  x  , uv.y -  y   ), 0).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    float3 upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;

   // imageStore(imgOutput, texelCoord, vec4(float3(upsample), 1));
    
    UAV[threadID] += float4(upsample, 1.0);
    
}