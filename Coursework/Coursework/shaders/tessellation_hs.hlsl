// Tessellation Hull Shader
// Prepares control points for tessellation
#include "common.hlsli"

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

// Interlocking tiles algorithm and distance based LOD finding adapted from Practical Rendering and Computation with Direct3D 11, chapter 9.
// Finding LODs based on screen area of the patch was done independently by me, inspired by a throwaway suggestion in the above book.

//---------- Distance based tessellation -----------

// Find centre of patch, to be compared with camera position
float3 FindPatchMiddle(float3 point0, float3 point1, float3 point2, float3 point3)
{
   return (point0 + point1 + point2 + point3) / 4.0f; 
}

// Scale distance to between 0 and 1, accounting for user defined min and max distances
float scaleDistance(float3 start, float3 end)
{
    float dist = distance(start, end);
    return saturate((dist - minDist) / (maxDist - minDist));
}

// Find LOD by lerping from min to max LOD, by the scaled distance
float FindPatchLodDist(float3 middlePoint)
{
    float scaledDist = scaleDistance(cameraPos.xyz, middlePoint);
    return lerp(minLOD, maxLOD, 1.0f - scaledDist);
}


//-------- Screen area based tessellation ---------

// Determine if the line between two points in screen space intersects with the edges of the screen
bool IntersectionTest(float4 point0, float4 point1)
{
    float xmin, xmax, ymin, ymax;
    if (point0.x > point1.x)
    {
        xmax = point0.x;
        xmin = point1.x;
    }
    else
    {
        xmax = point1.x;
        xmin = point0.x;
    }
    if (point0.y > point1.y)
    {
        ymax = point0.y;
        ymin = point1.y;
    }
    else
    {
        ymax = point1.y;
        ymin = point0.y;
    }
    
    //Check for intersects with x = 1.2f and x = -1.2f 
    //(1.2 instead of 1 to check a slightly larger area since the projected patch quad is just an estimation as we havent tessellated yet)
    if ((xmin <= 1.2f && xmax >= 1.2f) || (xmin <= -1.2f && xmax >= -1.2f))
    {
        if (ymin <= 1.2f && ymax >= -1.2f)
        {
            return true;
        }
    }
    //Check for intersects with y = 1.2f and y = -1.2f
    if ((ymin <= 1.2f && ymax >= 1.2f) || (ymin <= -1.2f && ymax >= -1.2f))
    {
        if (xmin <= 1.2f && xmax >= -1.2f)
        {
            return true;
        }
    }
    return false;
}

// Determine if any of the edges of the quad defined by the projected control points intersect with the edges of the screen 
bool PatchIntersects(float4 point0, float4 point1, float4 point2, float4 point3)
{
    if (IntersectionTest(point0, point1))
        return true;
    if (IntersectionTest(point0, point3))
        return true;
    if (IntersectionTest(point2, point1))
        return true;
    if (IntersectionTest(point2, point3))
        return true;
    return false;
}

// Find LOD based on area of quad defined by the patch control points in screen space
float FindPatchLodArea(float4 point0, float4 point1, float4 point2, float4 point3)
{
    float4 points[] = { point0, point1, point2, point3 };
    int offScreen = 0;
    
    // Figure out if the patch is entirely off-screen, if so we want just the lowest possible LOD (1)
    [unroll(4)]
    for (int i = 0; i < 4; i++)
    {
        points[i] /= points[i].w;
        if (points[i].x < -1.2f || points[i].x > 1.2f || points[i].y < -1.2f || points[i].y > 1.2f)
        {
            offScreen++;
        }
    }
    if (offScreen == 4)
    {
        //Control points are all outside of the viewport
        //If the patch doesn't intersect the viewport return 1
        if (!PatchIntersects(points[0], points[1], points[2], points[3]))
            return 1;
    }
    
    //Create weighting based on area of the patch
    float area = 0.5f * distance(points[0], points[1]) * distance(points[0], points[3]);
    area += 0.5f * distance(points[2], points[1]) * distance(points[2], points[3]);
    area = saturate(area * 10);
   
    return lerp(minLOD, maxLOD, area);
}


//--------- Both types of tessellation ----------

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 12> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    float LODs[5];
    
    // Find LODs using chosen method 
    if (distanceBased)
    {
        float3 middlePoints[] =
        {
            FindPatchMiddle(inputPatch[0].position, inputPatch[1].position, inputPatch[2].position, inputPatch[3].position), //Main quad
            FindPatchMiddle(inputPatch[1].position, inputPatch[4].position, inputPatch[5].position, inputPatch[2].position), //+x neighbour
            FindPatchMiddle(inputPatch[3].position, inputPatch[2].position, inputPatch[7].position, inputPatch[6].position), //+z neighbour
            FindPatchMiddle(inputPatch[8].position, inputPatch[0].position, inputPatch[3].position, inputPatch[9].position), //-x neighbour
            FindPatchMiddle(inputPatch[10].position, inputPatch[11].position, inputPatch[1].position, inputPatch[0].position) //-z neighbour
        };
        float array[] = {
            FindPatchLodDist(middlePoints[0]),
            FindPatchLodDist(middlePoints[1]),
            FindPatchLodDist(middlePoints[2]),
            FindPatchLodDist(middlePoints[3]),
            FindPatchLodDist(middlePoints[4])
        };
        LODs = array;
    }
    else
    {
        float array[] =
        {
            FindPatchLodArea(inputPatch[0].screenPos,  inputPatch[1].screenPos,  inputPatch[2].screenPos, inputPatch[3].screenPos),  //Main quad
            FindPatchLodArea(inputPatch[1].screenPos,  inputPatch[4].screenPos,  inputPatch[5].screenPos, inputPatch[2].screenPos),  //+x neighbour
            FindPatchLodArea(inputPatch[3].screenPos,  inputPatch[2].screenPos,  inputPatch[7].screenPos, inputPatch[6].screenPos),  //+z neighbour
            FindPatchLodArea(inputPatch[8].screenPos,  inputPatch[0].screenPos,  inputPatch[3].screenPos, inputPatch[9].screenPos),  //-x neighbour
            FindPatchLodArea(inputPatch[10].screenPos, inputPatch[11].screenPos, inputPatch[1].screenPos, inputPatch[0].screenPos) //-z neighbour
        };   
        LODs = array;
    }

    // Set the tessellation factors for the edges of the quad.
    output.edges[0] = min(LODs[0], LODs[3]);
	output.edges[1] = min(LODs[0], LODs[2]);
	output.edges[2] = min(LODs[0], LODs[1]);
    output.edges[3] = min(LODs[0], LODs[4]);

    // Set the tessellation factors for tessallating inside the quad.
	output.inside[0] = LODs[0];
    output.inside[1] = LODs[0];

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