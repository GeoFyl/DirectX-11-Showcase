#include "common.hlsli"


struct InputType
{
    float4 position : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float4 colour : COLOUR;
};

float4 main(InputType input, uint facing : SV_IsFrontFace) : SV_TARGET
{
    float3 normal = facing > 0 ? input.normal : -input.normal;
    
    switch (debugMode)
    {
        case 0: // No debugging 
            return CalculateFinalColour(normal, input.worldPos.xyz, input.colour);
            break;
        case 1: // Show only lighting
            return CalculateFinalColour(normal, input.worldPos.xyz);
            break;
        case 2: // Show normals
            return float4(normal.x, normal.y, normal.z, 1);
            break;
        default:
            return CalculateFinalColour(normal, input.worldPos.xyz, input.colour);
            break;
    }
}



