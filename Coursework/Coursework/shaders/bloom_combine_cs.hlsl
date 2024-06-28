Texture2D original : register(t0);
Texture2D blurred : register(t1);
RWTexture2D<float4> gOutput : register(u0);

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    // Combine original scene image with blurred highlights
    gOutput[dispatchThreadID.xy] = original[dispatchThreadID.xy] + blurred[dispatchThreadID.xy];
}