RWTexture2D<float4> UAV : register(u0);

Texture2D sourceTex : register(t0);


float gaussian(float2 i)
{
    return exp(-.5 * dot(i /= (5 * .25), i)) / sqrt(6.28 * (5 * .25) * (5 * .25));
}

float4 blur(Texture2D sp, float2 U)
{
    float4 O = float4(0,0,0,0);
    int s = 5;
    
    for (int i = 0; i < s*s; i++)
    {
        float2 d = float2(i % s, i / s) - s / 2.f;
        //O += gaussian(d) * sp.Load(int3(U + scale * d, 0));
        O += gaussian(d) * sp.Load(int3(U + d, 0));
    }
    
    return O/O.a;
}

[numthreads(32, 32, 1)]
void CSMain(uint2 threadID : SV_DispatchThreadID)
{
    UAV[threadID] = blur(sourceTex, threadID);    
}