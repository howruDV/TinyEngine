#include "struct.hlsli"
#include "global.hlsli"

struct PS_Input
{
    float4 vPosProj : SV_POSITION;
    float4 vCenterProj : POSITION;
    float2 vUV : TEXCOORD;
};

PS_Input main(VS_IN input)
{
    PS_Input output = (PS_Input) 0;
    output.vPosProj = mul(float4(input.vPos, 1.f), g_matWVP);
    output.vCenterProj = mul(float4(0.f, 0.f, 0.f, 1.f), g_matWVP);
    output.vUV = input.vUV;

    return output;
}