#include "common.hlsli"
#include "water.hlsli"

Texture2D waterTex : register(t0);
Texture2D normalMap : register(t1);
SamplerState sampleTex : register(s0);
SamplerState sampleNormals : register(s1);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 gridPos : POSITION;
    float3 worldPos : POSITION1;
};

void CalculateNormal(float3 pos, float wavelength, float steepness, float2 direction, inout float3 tangent, inout float3 binormal)
{
    //wavelength /= 2;
    //direction = normalize(float2(direction.x * cos(-1.2) - direction.y * sin(-1.2), direction.x * sin(-1.2) + direction.y * cos(-1.2)));
    direction = normalize(direction);
    float k = 2.f * 3.14159f / wavelength;
    float f = k * (dot(direction, pos.xz) - speed * time);
    tangent += float3(-direction.x * direction.x * (steepness * sin(f)), direction.x * (steepness * cos(f)), -direction.x * direction.y * (steepness * sin(f)));
    binormal += float3(-direction.x * direction.y * (steepness * sin(f)), direction.y * (steepness * cos(f)), - direction.y * direction.y * (steepness * sin(f)));
    //return normalize(cross(binormal, tangent));
}

float4 main(InputType input) : SV_TARGET
{
   // float3 tangent = float3(1, 0, 0);
    float3 tangent = float3(cos(1.2), 0, -sin(1.2));
    //float3 binormal = float3(0, 0, 1);
    float3 binormal = float3(sin(1.2), 0, cos(1.2));
    
    CalculateNormal(input.gridPos, wave1.x, wave1.y, float2(wave1.z, wave1.w), tangent, binormal);
    CalculateNormal(input.gridPos, wave2.x, wave2.y, float2(wave2.z, wave2.w), tangent, binormal);
    CalculateNormal(input.gridPos, wave3.x, wave3.y, float2(wave3.z, wave3.w), tangent, binormal);
    CalculateNormal(input.gridPos, wave4.x, wave4.y, float2(wave4.z, wave4.w), tangent, binormal);
    //float3 normal = normalize(cross(binormal, tangent) + normalMap.Sample(sampleNormals, input.tex * 10).xyz);
    float3 normal = normalize(normalMap.Sample(sampleNormals, input.tex).xyz * cross(binormal, tangent));
    //normal = normalize(mul(float4(normal, 1.0f), worldMatrix));
    
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    //float4 textureColour = waterTex.Sample(sampleTex, input.tex * 5);
    float4 textureColour = float4(0.349, 0.624, 0.761, 1);
    
    switch (debugMode)
    {
        case 0:
            return CalculateFinalColour(normal, input.worldPos, textureColour);
            break;
        case 1:
            return CalculateFinalColour(normal, input.worldPos);
            break;
        case 2:
            return float4(normal.x, normal.y, normal.z, 0);
            break;
        default:
            return CalculateFinalColour(normal, input.worldPos, textureColour);
            break;
    }
}



