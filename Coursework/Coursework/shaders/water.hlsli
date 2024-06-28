
cbuffer WaterBuffer : register(b3)
{
    float4 wave1; //Wavelength, Steepness, Direction.x, Direction.y
    float4 wave2;
    float4 wave3;
    float4 wave4;
    float time;
    float speed;
    float2 padding2;
}


// Modify position to account for the passed in wave data
// Based on method from: https://catlikecoding.com/unity/tutorials/flow/waves/
float3 GetNewWaterPos(float3 pos, float wavelength, float steepness, float2 direction)
{
    float3 newPos = pos;
    direction = normalize(direction);
    float k = 2.f * 3.14159f / wavelength;
    float f = k * (dot(direction, pos.xz) - speed * time);
    float a = steepness / k;
    newPos.x = direction.x * (a * cos(f));
    newPos.y = a * sin(f);
    newPos.z = direction.y * (a * cos(f));
    return newPos;
}
