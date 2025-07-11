// texture vertex shader
// Basic shader for rendering textured geometry

#include "common.hlsli"

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 worldPos : POSITION;
};

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.worldPos = output.position;
    output.position = ViewProject(output.position);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

    output.normal = input.normal;

	return output;
}