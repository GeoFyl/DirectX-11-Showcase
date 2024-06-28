#include "common.hlsli"
#include "water.hlsli"

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

    output.position = input.position;
	
    // If tessellation is based on screen area, find the right position and project the control point to screen space to be used later  
    if (!distanceBased)
    {
        output.screenPos = input.position;
        
        // Modify the vertex position with gerstner waves
        output.screenPos.xyz += GetNewWaterPos(output.screenPos.xyz, wave1.x, wave1.y, float2(wave1.z, wave1.w));
        output.screenPos.xyz += GetNewWaterPos(output.screenPos.xyz, wave2.x, wave2.y, float2(wave2.z, wave2.w));
        output.screenPos.xyz += GetNewWaterPos(output.screenPos.xyz, wave3.x, wave3.y, float2(wave3.z, wave3.w));
        output.screenPos.xyz += GetNewWaterPos(output.screenPos.xyz, wave4.x, wave4.y, float2(wave4.z, wave4.w));
    
	    // Calculate the screenPos of the vertex against the world, view, and projection matrices.
        output.screenPos = WorldViewProject(output.screenPos);
    }
	
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	return output;
}