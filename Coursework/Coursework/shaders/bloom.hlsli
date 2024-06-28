#define BLOOM_KERNEL_ARRAY_SIZE 400

#define N 256
#define CacheSize (N + BLOOM_KERNEL_ARRAY_SIZE - 2)
groupshared float4 gCache[CacheSize];

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

cbuffer blurBuffer : register(b0)
{
    // Packing an array of floats explained by: https://www.gamedev.net/forums/topic/590226-constant-buffer-with-an-array-of-floats/4745908/
    float4 kernel[BLOOM_KERNEL_ARRAY_SIZE / 4 + !!(BLOOM_KERNEL_ARRAY_SIZE % 4)];
    float radius;  
    float threshold;
}

// Filter out low-brightness pixels as we are only blurring the bright parts of the image
float4 BrightnessFilter(float4 colour)
{
    float brightness = max(max(colour.r, colour.g), colour.b);
    if (brightness >= threshold)
        return colour;
    else
        return float4(0, 0, 0, 1);
}

// Apply gaussian blur to pixel
float4 Blur(int ID)
{
    float4 blurColor = float4(0, 0, 0, 0);
	
    for (int i = -radius; i <= radius; ++i)
    {
        int k = ID + radius + i;

        blurColor += ((float[4]) (kernel[(i + radius) / 4]))[(i + radius) % 4] * gCache[k];
        
    }
    
    return blurColor;
}