// Horizontal compute blurr, based on Frank Luna's example. Adapted from the lab examples.
#include "bloom.hlsli"

[numthreads(N, 1, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if (groupThreadID.x < radius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = max(dispatchThreadID.x - radius, 0);
        gCache[groupThreadID.x] = BrightnessFilter(gInput[int2(x, dispatchThreadID.y)]);
    }
	if (groupThreadID.x >= N - radius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = min(dispatchThreadID.x + radius, gInput.Length.x - 1);
        gCache[groupThreadID.x + 2 * radius] = BrightnessFilter(gInput[int2(x, dispatchThreadID.y)]);
    }

	// Clamp out of bound samples that occur at image borders.
    gCache[groupThreadID.x + radius] = BrightnessFilter(gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)]);

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	// Apply blur
    float4 blurColour = Blur(groupThreadID.x);

	gOutput[dispatchThreadID.xy] = blurColour;
}