// Vertical compute blurr, based on Frank Luna's example. Adapted from the lab examples.
#include "bloom.hlsli"

[numthreads(1, N, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
    if (groupThreadID.y < radius)
    {
		// Clamp out of bound samples that occur at image borders.
        int y = max(dispatchThreadID.y - radius, 0);
        gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
    }
    if (groupThreadID.y >= N - radius)
    {
		// Clamp out of bound samples that occur at image borders.
        int y = min(dispatchThreadID.y + radius, gInput.Length.y - 1);
        gCache[groupThreadID.y + 2 * radius] = gInput[int2(dispatchThreadID.x, y)];
    }

	// Clamp out of bound samples that occur at image borders.
    gCache[groupThreadID.y + radius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];

	// Wait for all threads to finish.
    GroupMemoryBarrierWithGroupSync();

	// Apply blur
    float4 blurColour = Blur(groupThreadID.y);

    gOutput[dispatchThreadID.xy] = blurColour;
}