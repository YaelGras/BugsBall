RWTexture2D<float4> UAV : register(u0);
Texture2D           source; // we need srv for sampling

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
    float4 firstPass;
};

SamplerState ss;


[numthreads(8, 8, 1)]
void CSMain(uint2 threadID : SV_DispatchThreadID)
{
    // Find the correct sample step disatnce
    int width, height;
    
    source.GetDimensions(width, height);
    float2 srcResolution = float2(width, height);
    
    
    UAV.GetDimensions(width, height);
    float2 targetResolution = float2(width, height);
    
    
    
    float2 uv = float2(threadID.x / (float) (width - 1), threadID.y / (float) (height - 1));
   // uv = float2(threadID.x / (8 * firstPass.y), threadID.y / (4 * firstPass.z));
    
    float2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    

    
    float3 a = source.SampleLevel(ss, float2(uv.x - 2 * x, uv.y + 2 * y     ),0).rgb;
    float3 b = source.SampleLevel(ss, float2(uv.x        , uv.y + 2 * y     ),0).rgb;
    float3 c = source.SampleLevel(ss, float2(uv.x + 2 * x, uv.y + 2 * y     ),0).rgb;  
    
    float3 d = source.SampleLevel(ss, float2(uv.x - 2 * x, uv.y             ),0).rgb;
    float3 e = source.SampleLevel(ss, float2(uv.x        , uv.y             ),0).rgb;
    float3 f = source.SampleLevel(ss, float2(uv.x + 2 * x, uv.y             ),0).rgb;
    
    float3 g = source.SampleLevel(ss, float2(uv.x - 2 * x, uv.y - 2 * y     ),0).rgb;
    float3 h = source.SampleLevel(ss, float2(uv.x        , uv.y - 2 * y     ),0).rgb;
    float3 i = source.SampleLevel(ss, float2(uv.x + 2 * x, uv.y - 2 * y     ),0).rgb;
    
    float3 j = source.SampleLevel(ss, float2(uv.x - x, uv.y + y             ),0).rgb;
    float3 k = source.SampleLevel(ss, float2(uv.x + x, uv.y + y             ),0).rgb;
    float3 l = source.SampleLevel(ss, float2(uv.x - x, uv.y - y             ),0).rgb;
    float3 m = source.SampleLevel(ss, float2(uv.x + x, uv.y - y             ),0).rgb;
    
    
    // Wieghts 
    float4 downsample = float4(0,0,0,1);
    downsample.rgb = e * 0.125;
    downsample.rgb += (a + c + g + i) * 0.03125;
    downsample.rgb += (b + d + f + h) * 0.0625;
    downsample.rgb += (j + k + l + m) * 0.125;


    if (firstPass.r > 1)
    {
        downsample.rgb = max(float3(0,0,0), downsample.rgb - 1);
    }

    
    UAV[threadID] = downsample;
    
}