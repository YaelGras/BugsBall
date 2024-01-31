
Texture2D tex_slots[8];
Texture2D normal_slots[8];
Texture2D ao_slots[8];
Texture2D spec_slots[8];

float3 sunDir = normalize(float3(27.0f, 276.f, -508.0f)); // la position de la source d’éclairage (Point) SamplerState SampleState; // l’état de sampling

SamplerState billboardSampler
{
    AddressU = wrap;
    AddressV = clamp;
    AddressW = clamp;
    Filter = MIN_MAG_MIP_LINEAR;
}; 

cbuffer CameraParams
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4 position;
};

struct VertexInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
    
    float3 instancePos : INSTANCE_POS;
    float3 instanceNormal : INSTANCE_NORMAL;
    float2 instanceSize : INSTANCE_SIZE;
    float4 instanceCol : INSTANCE_COLOR;
    float4 instanceUV : INSTANCE_UV;
    float instanceTexID : INSTANCE_TEXID;


};

struct VertexOut
{
    float4 pos : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    uint texId : TEXCOORD1;
    float4 color : TEXCOORD2;
    float4 worldPos : TEXCOORD3;
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


VertexOut billboardVS(VertexInput VSin)
{
    VertexOut vso;
    
    float3 up = float3(0.001, 0.999, 0);
    
    float3 v = normalize(cross(VSin.instanceNormal, up));
    float3 n = normalize(cross(VSin.instanceNormal, v));
    
    
    vso.pos = float4((VSin.pos.xy * VSin.instanceSize), 0, 1);
    vso.pos = float4(vso.pos.x * v + vso.pos.y * n + vso.pos.z * VSin.instanceNormal, 1.0);
    vso.pos += float4(VSin.instancePos, 0);
    
    vso.uv = VSin.uv * VSin.instanceUV.zw + VSin.instanceUV.xy;
    vso.uv = float2(0, 1) - (vso.uv);
    vso.normal = normalize(VSin.instanceNormal);
    vso.texId = uint(VSin.instanceTexID);
    vso.color = VSin.instanceCol;
    vso.worldPos = vso.pos;
    vso.pos = mul(mul(projMatrix, viewMatrix), vso.pos);
    
    return vso;
}

PSOut billboardPS(VertexOut vsin) : SV_Target
{
   PSOut pso;
    float4 texSample = float4(0, 0, 0, 1);
    float ambiantOcclusion = float4(0, 0, 0, 1);
    float3 tempNormal = vsin.normal;
    
    for (uint i = 0; i < 8; i++)
    {
        if (i == vsin.texId)
        {
            texSample = tex_slots[i].Sample(billboardSampler, vsin.uv);
            ambiantOcclusion = ao_slots[i].Sample(billboardSampler, vsin.uv);
            tempNormal = normal_slots[i].Sample(billboardSampler, vsin.uv).xyz;
        }

    }
    if (texSample.a <= 0.001f)
        discard;
    
    


    
    float3 prout = float3(1, 2, 3);
    float3 tan = normalize(cross(prout, vsin.normal));
    float3 bi = normalize(cross(tan, vsin.normal));
    
    tempNormal = normalize(tempNormal * 2.0 - 1.0);
    pso.Normal = float4(tempNormal.x * tan + vsin.normal * tempNormal.y + tempNormal.z * bi, 0.0f);
    pso.Normal = float4(normalize(pso.Normal.xyz), 1.0f);
    
    float3 L = normalize(sunDir);
    float3 V = normalize(position - vsin.worldPos);
    float3 H = normalize(L + V);
    float diff = saturate(dot(L, pso.Normal));
    float S = pow(max(dot(pso.Normal, H), 0), 32.f);
    
    pso.Albedo = texSample * vsin.color * pow(ambiantOcclusion, 2.0f);
    pso.Albedo.a = 1.0f;
    pso.Position = vsin.worldPos;
    
    pso.Specular = float4(S, S, S, 1);
    pso.ambiantOcclusion = float4(0, 0, 0, 1);
    pso.roughness = float4(0, 0, 0, 1);
    
    return pso;
}


technique11 sprite2D
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_5_0, billboardVS()));
        SetPixelShader(CompileShader(ps_5_0, billboardPS()));
        SetGeometryShader(NULL);
    }
}