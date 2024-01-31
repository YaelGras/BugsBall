
Texture2D tex_slots[8];
Texture2D normal_slots[8]; // unused
Texture2D ao_slots[8]; // unused

SamplerState SampleState
{
    AddressU = wrap;
    AddressV = wrap;
    AddressW = wrap;
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
    vso.pos = mul(mul(projMatrix, viewMatrix), vso.pos);
    
    vso.uv = VSin.uv * VSin.instanceUV.zw + VSin.instanceUV.xy;
    vso.uv = float2(0, 1) - (vso.uv);
    vso.normal = VSin.instanceNormal;
    vso.texId = uint(VSin.instanceTexID);
    vso.color = VSin.instanceCol;
    
    return vso;
}

float4 billboardPS(VertexOut vsin) : SV_Target
{
    float4 texSample = float4(0, 0, 0, 1);
    
    for (uint i = 0; i < 8; i++)
    {
        if (i == vsin.texId)
        {
            texSample = tex_slots[i].Sample(SampleState, vsin.uv);
        }

    }
    return texSample * vsin.color;
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