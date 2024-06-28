// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
#include "common.hlsli"
#include "water.hlsli"

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 gridPos : POSITION;
    float3 worldPos : POSITION1;
};

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float3 vertexPosition;
    OutputType output;
 
    // Determine the new texture coordinate.
    float texU = lerp(patch[0].tex.x, patch[1].tex.x, uvCoord.x);
    float texV = lerp(patch[3].tex.y, patch[0].tex.y, uvCoord.y);    
    output.tex = float2(texU, texV);    
    
    // Determine the position of the new vertex.
    float3 v1 = lerp(patch[3].position, patch[0].position, uvCoord.y);
    float3 v2 = lerp(patch[2].position, patch[1].position, uvCoord.y);
    vertexPosition = lerp(v1, v2, uvCoord.x);
    
    output.gridPos = vertexPosition;
    
    // Modify the vertex position with gerstner waves
    vertexPosition.xyz += GetNewWaterPos(vertexPosition.xyz, wave1.x, wave1.y, float2(wave1.z, wave1.w));
    vertexPosition.xyz += GetNewWaterPos(vertexPosition.xyz, wave2.x, wave2.y, float2(wave2.z, wave2.w));
    vertexPosition.xyz += GetNewWaterPos(vertexPosition.xyz, wave3.x, wave3.y, float2(wave3.z, wave3.w));
    vertexPosition.xyz += GetNewWaterPos(vertexPosition.xyz, wave4.x, wave4.y, float2(wave4.z, wave4.w));
		    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.worldPos = output.position;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    return output;
}

