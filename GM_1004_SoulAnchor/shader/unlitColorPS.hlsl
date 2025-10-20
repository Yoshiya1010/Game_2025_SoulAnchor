
#include "common.hlsl"


Texture2D		g_Texture : register(t0);
SamplerState	g_SamplerState : register(s0);


void main(in PS_IN In, out float4 outColor : SV_Target)
{

    outColor.rgb = In.Diffuse.rgb * 0.7;
    outColor.a = In.Diffuse.a;

}
