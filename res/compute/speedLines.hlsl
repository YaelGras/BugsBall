// stolen from https://www.shadertoy.com/view/ctdfzN

// Modified fork of "Speed lines" by "Hadyn": https://www.shadertoy.com/view/4dSyWK

RWTexture2D<float4> UAV : register(u0);

cbuffer cameraBuffer : register(b0)
{
    float4 camPos; // Example parameter
    float4 cameraRange;
    float4x4 viewProj;
    float4x4 view;
};


/////////////////////////////////////////////

#define RADIUS 12.0
#define EDGE 0.3

cbuffer linesBuffer
{
    float radius;
    float edges;
    float opacity;
    float deltaTime;
    
};


/////////////////////////////////////////////

float3 random3(float3 c)
{
    float j = 4096.0 * sin(dot(c, float3(17.0, 59.4, 15.0)));
    float3 r;
    r.z = frac(512.0 * j);
    j *= .125;
    r.x = frac(512.0 * j);
    j *= .125;
    r.y = frac(512.0 * j);
    return r - 0.5;
}

float simplex3d(float3 p)
{
    float3 s = floor(p + dot(p, float3(0.3333333, 0.3333333, 0.3333333)));
    float3 x = p - s + dot(s, float3(0.1666667, 0.1666667, 0.1666667));
    float3 e = step(float3(0,0,0), x - x.yzx);
    float3 i1 = e * (1.0 - e.zxy);
    float3 i2 = 1.0 - e.zxy * (1.0 - e);
    float3 x1 = x - i1 + 0.1666667;
    float3 x2 = x - i2 + 2.0 * 0.1666667;
    float3 x3 = x - 1.0 + 3.0 * 0.1666667;
    float4 w, d;
    w.x = dot(x, x);
    w.y = dot(x1, x1);
    w.z = dot(x2, x2);
    w.w = dot(x3, x3);
    w = max(0.6 - w, 0.0);
    d.x = dot(random3(s), x);
    d.y = dot(random3(s + i1), x1);
    d.z = dot(random3(s + i2), x2);
    d.w = dot(random3(s + 1.0), x3);
    w *= w;
    w *= w;
    d *= w;
    return dot(d, float4(52.0, 52.0, 52.0, 52.0));
}

[numthreads(8, 8, 1)]
void CSMain(uint2 threadID : SV_DispatchThreadID)
{
   
    int width, height;
    
    UAV.GetDimensions(width, height);
    float2 srcResolution = float2(width, height);
    
    float2 uv = float2(threadID.x / (float) width, threadID.y / (float) height) * 2 - 1.f ;
    
    float time = deltaTime * 2.;
    float scale = 50.0;
    
    float2 p = float2(0.5 * width / height, 0.5) + normalize(uv) * min(length(uv), 0.05);
    float3 p3 = scale * 0.25 * float3(p.xy, 0) + float3(0, 0, time * 0.025);
    float noise = simplex3d(p3 * 32.0) * 0.5 + 0.5;
    float dist = abs(clamp(length(uv) / radius, 0.0, 1.0) * noise * 2. - 1.);
    float stepped = smoothstep(edges - .5, edges + .5, noise * (1.0 - pow(dist, 4.0)));
    float final = smoothstep(edges - 0.05, edges + 0.05, noise * stepped);
    
    UAV[threadID] += float4(float3(opacity, opacity, opacity) * final, opacity);
    
}