// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
#include "common.hlsli"
#include "terrain.hlsli"

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
    float3 worldPos : POSITION;
    float4 localPos : POSITION2;
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
    
    vertexPosition.y = GetHeight(output.tex);
    
    output.localPos = float4(vertexPosition, 1.0f);
		    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(output.localPos, worldMatrix);
    output.worldPos = output.position;
    output.position = ViewProject(output.position);

    return output;
}

