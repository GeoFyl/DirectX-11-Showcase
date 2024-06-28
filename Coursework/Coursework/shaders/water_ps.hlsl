#include "common.hlsli"
#include "water.hlsli"

Texture2D normalMap : register(t0);
SamplerState sampleNormals : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 gridPos : POSITION;
    float3 worldPos : POSITION1;
};

// Modify normals to account for the passed in wave data
// Based on method from: https://catlikecoding.com/unity/tutorials/flow/waves/
void CalculateNormal(float3 pos, float wavelength, float steepness, float2 direction, inout float3 tangent, inout float3 binormal)
{
    direction = normalize(direction);
    float k = 2.f * 3.14159f / wavelength;
    float f = k * (dot(direction, pos.xz) - speed * time);
    tangent += float3(-direction.x * direction.x * (steepness * sin(f)), direction.x * (steepness * cos(f)), -direction.x * direction.y * (steepness * sin(f)));
    binormal += float3(-direction.x * direction.y * (steepness * sin(f)), direction.y * (steepness * cos(f)), - direction.y * direction.y * (steepness * sin(f)));
}

float4 main(InputType input) : SV_TARGET
{
    float3 tangent = float3(cos(1.2), 0, -sin(1.2));
    float3 binormal = float3(sin(1.2), 0, cos(1.2));
    
    CalculateNormal(input.gridPos, wave1.x, wave1.y, float2(wave1.z, wave1.w), tangent, binormal);
    CalculateNormal(input.gridPos, wave2.x, wave2.y, float2(wave2.z, wave2.w), tangent, binormal);
    CalculateNormal(input.gridPos, wave3.x, wave3.y, float2(wave3.z, wave3.w), tangent, binormal);
    CalculateNormal(input.gridPos, wave4.x, wave4.y, float2(wave4.z, wave4.w), tangent, binormal);

    float3 normal = normalize(normalMap.Sample(sampleNormals, input.tex).xyz * cross(binormal, tangent));

    float4 textureColour = float4(0.047, 0.169, 0.149, 0.4f);
    
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



