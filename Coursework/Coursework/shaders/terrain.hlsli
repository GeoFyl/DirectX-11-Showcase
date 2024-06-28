SamplerState sampleHeight : register(s0);
Texture2D heightMap : register(t0);

cbuffer TerrainBuffer : register(b3)
{
    float heightScale;
    float uvSteps;
    float2 padding2;
}

// Sample the heightmap and scale to find height
float GetHeight(float2 uv)
{
    return heightMap.SampleLevel(sampleHeight, uv, 0).r * heightScale;
}

// Calculate the normal at given uv based on the heightmap
float3 CalculateHeightMapNormal(float2 uv)
{
    // How far to sample the heightmap from the centre uv coord (heightmap is 4096x4096 pixels).
    float sampleStep = uvSteps / 4096.f;
    
    // Distance from centre to the next sampled location in world space
    float distance = sampleStep * heightScale;
	
    // Create tangents pointing up, down, left and right from the centre
    float3 centre = (0, GetHeight(uv), 0);
    
    float3 upTangent = float3(0, GetHeight(float2(uv.x, uv.y - sampleStep)), distance) - centre;
    float3 downTangent = float3(0, GetHeight(float2(uv.x, uv.y + sampleStep)), -distance) - centre;
    float3 rightTangent = float3(distance, GetHeight(float2(uv.x + sampleStep, uv.y)), 0) - centre;
    float3 leftTangent = float3(-distance, GetHeight(float2(uv.x - sampleStep, uv.y)), 0) - centre;
	
    // Find normal from cross products of the tangents
    float3 normal = float3(cross(upTangent, rightTangent) + cross(rightTangent, downTangent) + cross(downTangent, leftTangent) + cross(leftTangent, upTangent));
    normal /= 4.f;
	
    return normalize(normal);
}