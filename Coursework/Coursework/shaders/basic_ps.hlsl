cbuffer colourBuffer : register(b0)
{
    float4 colour;
}

struct InputType
{
	float4 position : SV_POSITION;
};

float4 main(InputType input) : SV_TARGET
{
    return colour;
}



