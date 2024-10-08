#include "struct.hlsli"
#include "global.hlsli"
#include "func.hlsli"

PS_IN main(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
   
    float3 PrevPos = input.vPos;
    if (g_iAnim)
    {
        Skinning(input.vPos, PrevPos, input.vTangent, input.vBitangent, input.vNormal
              , input.vWeights, input.vIndices, 0);
    }
    
    float4 normal = float4(input.vNormal, 0.0);
    output.vNormalWorld = mul(normal, g_matWorldInvTranspose).xyz;
    output.vNormalWorld = normalize(output.vNormalWorld);
    
    float4 pos = float4(input.vPos, 1.0);
    output.vPosProj = mul(pos, g_matWorld);
    output.vPosWorld = mul(pos, g_matWorld).xyz; // 월드 위치 따로 저장
    
    // 카메라와의 거리와 thickness 값으로 외곽선 범위 만큼 크기 확장
    float dist = length(output.vPosWorld - g_eyeWorld);
    float thickness = 0.005f;
    
    output.vPosProj += float4(output.vNormalWorld * dist * thickness, 0.0);
    output.vPosProj = mul(output.vPosProj, g_matView);
    output.vPosProj = mul(output.vPosProj, g_matProj);

    return output;
}