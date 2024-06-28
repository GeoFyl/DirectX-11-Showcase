#include "common.hlsli"
#include "terrain.hlsli"

SamplerState sampleTex : register(s1);
Texture2D grassTex : register(t1);
Texture2D grassNormal : register(t2);
Texture2D mudTex : register(t3);
Texture2D mudNormal : register(t4);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 worldPos : POSITION;
    float4 depthPos : POSITION1;
};

float4 main(InputType input) : SV_TARGET
{
    // Calculate the normal using the heightmap
    float3 normal = CalculateHeightMapNormal(input.tex);

    float4 textureColour;
    
    // Texture the terrain based on the gradient (using the normals y component)
    // Also combine the normal with values sampled from the normal map(s)
    if (normal.y < 0.2 || ((input.worldPos.x > 18 && input.worldPos.x < 53) && (input.worldPos.z > 30 && input.worldPos.z < 55) && normal.y > 0.4))
    {
        //Mud on steep areas and in the water
        textureColour = mudTex.Sample(sampleTex, input.tex * 15);
        normal *= mudNormal.Sample(sampleTex, input.tex * 15);
    }
    else if (normal.y > 0.8)
    {
        // Grass on flat areas
        textureColour = grassTex.Sample(sampleTex, input.tex * 10);
        normal *= grassNormal.Sample(sampleTex, input.tex * 10);
    }
    else
    {
        // In areas that arent super steep nor flat, lerp between the two textures
        float4 grass = grassTex.Sample(sampleTex, input.tex * 10);
        float4 grassN = grassNormal.Sample(sampleTex, input.tex * 10);
        float4 mud = mudTex.Sample(sampleTex, input.tex * 15);
        float4 mudN = mudNormal.Sample(sampleTex, input.tex * 15);
        textureColour = lerp(mud, grass, normal.y);
        normal *= lerp(mudN, grassN, normal.y);
    }
    
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