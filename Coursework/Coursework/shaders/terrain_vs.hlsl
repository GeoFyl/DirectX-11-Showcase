#include "common.hlsli"
#include "terrain.hlsli"

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float3 position : POSITION;
    float4 screenPos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

OutputType main(InputType input)
{
    OutputType output;

	 // Pass the vertex position into the hull shader.
    output.position = input.position;

    // If tessellation is based on screen area, find the right height and project the control point to screen space to be used later   
    if (!distanceBased)
    {
        // Calculate the position of the vertex against the world, view, and projection matrices.
        output.screenPos = input.position;
        output.screenPos.y = GetHeight(input.tex);
        output.screenPos = WorldViewProject(output.screenPos);
    }
    
    output.tex = input.tex;
    
    return output;
}
