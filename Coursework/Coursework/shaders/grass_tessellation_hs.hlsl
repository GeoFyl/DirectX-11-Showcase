// Tessellation Hull Shader
// Prepares control points for tessellation
cbuffer DensityBuffer : register(b0)
{
    float density;
}

struct InputType
{
    float3 position : POSITION;
    float4 screenPos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 12> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    // Set the tessellation factors for the edges of the quad.
    output.edges[0] = density;
	output.edges[1] = density;
	output.edges[2] = density;
    output.edges[3] = density;

    // Set the tessellation factors for tessallating inside the quad.
	output.inside[0] = density;
    output.inside[1] = density;

    return output;
}


[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 12> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;


    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;
    
    output.tex = patch[pointId].tex;

    return output;
}