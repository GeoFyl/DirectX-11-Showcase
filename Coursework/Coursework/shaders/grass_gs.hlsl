#include "common.hlsli"
#include "terrain.hlsli"

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPos : POSITION;
    float4 localPos : POSITION2;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float4 colour : COLOUR;
};

//------------------------------ Method adapted (with simplifications) from: https://roystan.net/articles/grass-shader/ ----------------------------------

// Simple noise function, sourced from http://answers.unity.com/answers/624136/view.html
// Returns a number in the 0...1 range.
float rand(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 53.539))) * 43758.5453);
}

// Construct a rotation matrix that rotates around the provided axis, sourced from:
// https://gist.github.com/keijiro/ee439d5e7388f3aafc5296005c8c3f33
float3x3 RotationMatrix(float angle, float3 axis)
{
    float c, s;
    sincos(angle, s, c);

    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return float3x3(
			t * x * x + c, t * x * y - s * z, t * x * z + s * y,
			t * x * y + s * z, t * y * y + c, t * y * z - s * x,
			t * x * z - s * y, t * y * z + s * x, t * z * z + c
			);
}

[maxvertexcount(7)]
void main(triangle InputType input[3], inout TriangleStream<OutputType> triangleStream)
{
    // Calculate the normal using the heightmap
    float3 normal = CalculateHeightMapNormal(input[0].tex) + CalculateHeightMapNormal(input[1].tex) + CalculateHeightMapNormal(input[2].tex);
    normal /= 3;

    // We only want grass to grow at fairly flat areas and not in the water
    if (normal.y >= 0.6 && ((input[0].worldPos.x < 15 || input[0].worldPos.x > 53) || (input[0].worldPos.z < 30 || input[0].worldPos.z > 55)))
    {
        OutputType output;
        
        // Blade of grass will have a colour gradient from bottom to top
        float4 bottomColour = float4(0.4, 0.51, 0.176, 1);
        float4 topColour = float4(0.506, 0.659, 0.196, 1);        
        
        // Find a tangent and binormal
        // Method from: https://www.gamedev.net/forums/topic/552411-calculate-tangent-from-normal/4552508/ 
        float3 t1 = cross(normal, float3(0, 0, 1));
        float3 t2 = cross(normal, float3(0, 1, 0));
        float3 tangent;
        if (length(t1) > length(t2))
        {
            tangent = normalize(t1);
        }
        else
        {
            tangent = normalize(t2);
        }
        float3 binormal = normalize(cross(normal, tangent));
        
        // Blades of grass are constructed in tangent space, so need to be able to transform them to local space
        float3x3 tangentToLocal = float3x3(
	        tangent.x, binormal.x, normal.x,
	        tangent.y, binormal.y, normal.y,
	        tangent.z, binormal.z, normal.z
	    );
        
        float3x3 rotation = RotationMatrix(rand(input[0].worldPos) * 6.28318530718, normal); // Rotation to face the blade of grass in a random direction
        float3x3 bend = RotationMatrix(rand(input[0].worldPos.zzx) * 0.2 * 1.57079632679, float3(-1, 0, 0)); // How much the blade of grass leans over
        
        // Construct transformation matrix
        float3x3 transform = mul(mul(rotation, tangentToLocal), bend);
        
        // Blade of grass will be placed at the centre of the input triangle
        float3 pos = input[0].worldPos + input[1].worldPos + input[2].worldPos;
        pos /= 3;
        
        // Random dimensions 
        float height = (rand(pos.zyx) * 2 - 1) * 0.3 + 0.5;
        float width = (rand(pos.xzy) * 2 - 1) * 0.02 + 0.05;
        float forward = rand(pos.yyz) * 0.38f;  // How much the blade of grass curves
        
        // Blade of grass is made of three segments tapering from bottom to top
        
        // Construct bottom two segments (two triangles each)
        for (int i = 0; i < 3; i++)
        { 
            float t = i / 3.f;
            float segmentHeight = height * t;
            float segmentWidth = width * (1 - t);
            float segmentForward = pow(t, 2) * forward;
            
            output.colour = lerp(bottomColour, topColour, t);                

            output.normal = mul(transform, float3(0, -1, segmentForward));
            
            output.worldPos = float4(pos + mul(transform, float3(segmentWidth, segmentForward, segmentHeight)), 1);
            output.position = ViewProject(float4(output.worldPos, 1));
            triangleStream.Append(output);
            
            output.worldPos = float4(pos + mul(transform, float3(-segmentWidth, segmentForward, segmentHeight)), 1);
            output.position = ViewProject(float4(output.worldPos, 1));
            triangleStream.Append(output);
        }
        // Add final vertex at tip
        output.colour = topColour;
        output.normal = mul(transform, float3(0, -1, forward));
        output.worldPos = float4(pos + mul(transform, float3(0, forward, height)), 1);
        output.position = ViewProject(float4(output.worldPos, 1));
        triangleStream.Append(output);
    }     
}