
#include"common.hlsl"

struct PS_IN_LINE
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
};

float4 main(PS_IN_LINE input) : SV_Target
{
    return input.Color;
}