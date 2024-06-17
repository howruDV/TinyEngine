#include "struct.hlsli"
#include "global.hlsli"
#include "func.hlsli"

PS_IN main(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
    if (g_iAnim)
    {
        Skinning(input.vPos, input.vTangent, input.vBitangent, input.vNormal
              , input.vWeights, input.vIndices, 0);
    }
    
    output.vPosProj = mul(float4(input.vPos, 1.0f), g_matWVP);

    return output;
}