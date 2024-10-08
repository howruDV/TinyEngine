
#include "struct.hlsli"
#include "global.hlsli"

#define ALPHADISCARD g_float_0
#define STATICCOLOR g_vec4_0

#define FLOAT_VALUE1 g_float_1

#define FADEINOUT g_int_3

float4 main(PS_IN _in) : SV_Target
{
    float4 output = (float4) 0.f;
    
    // Texture Exist
    if (g_btex_0)
    {
        output = g_tex_0.Sample(g_LinearWrapSampler, _in.vUV0);
        if (output.r >= ALPHADISCARD)
            discard;
    }
    
    output = STATICCOLOR;
    
    if (FADEINOUT)
    {
        output.a = FLOAT_VALUE1;
    }
    else
    {
        output.a = 1.f;
    }
    
    return output;
}