// Texture pixel/fragment shader
// Basic fragment shader for rendering textured geometry

// Texture and sampler registers
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer typeBuffer : register(b0)
{
    float4 type;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    float4 colour = texture0.Sample(Sampler0, input.tex);
    if (type.x == 1) return colour.rrra; // This is a shadow map, return greyscale
    else return colour;

}