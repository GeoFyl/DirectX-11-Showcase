// Texture pixel/fragment shader
// Basic fragment shader for rendering textured geometry
#include "common.hlsli"


// Texture and sampler registers
Texture2D texture0 : register(t0);
Texture2D normalMap : register(t1);
SamplerState Sampler0 : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 worldPos : POSITION;
};

float4 main(InputType input) : SV_TARGET
{
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    float4 textureColour = texture0.Sample(Sampler0, input.tex);
    
    // Combine input normal with normal from normal map
    float3 normal = input.normal * normalMap.Sample(Sampler0, input.tex);

    switch (debugMode)
    {
        case 0: // No debugging 
            return CalculateFinalColour(normal, input.worldPos.xyz, textureColour);
            break;
        case 1: // Show only lighting
            return CalculateFinalColour(normal, input.worldPos.xyz);
            break;
        case 2: // Show normals
            return float4(normal.x, normal.y, normal.z, 1);
            break;
        default:
            return CalculateFinalColour(normal, input.worldPos.xyz, textureColour);
            break;
    }
}