// Functions used by multiple shaders

#define NUM_LIGHTS 4 // How many lights are in use 

SamplerComparisonState shadowSampler : register(s2);
Texture2D depthMapTexture[NUM_LIGHTS * 6] : register(t5); // Each light gets 6 shadow maps as it may be a point light

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer LightsBuffer : register(b1)
{
    float4 lightAmbient[NUM_LIGHTS];
    float4 lightDiffuse[NUM_LIGHTS];
    float4 lightSpecular[NUM_LIGHTS];
    float4 lightDirection[NUM_LIGHTS];
    float4 lightPosition[NUM_LIGHTS];
    float4 lightAttenFac[NUM_LIGHTS];
    float4 lightAngleExpSpecpow[NUM_LIGHTS]; // spotlight angle, spotlight exponent, specular power
    matrix lightViews[NUM_LIGHTS * 6];
    matrix lightProjection[NUM_LIGHTS];
    float3 cameraPos;
    float debugMode;
    float shadowBias;
    float3 padding1;
}

cbuffer TessellationBuffer : register(b2)
{
    float minLOD;
    float maxLOD;
    float minDist;
    float maxDist;
    float distanceBased;
    float3 padding0;
}

// Apply world, view and projection matrices
float4 WorldViewProject(float4 pos)
{
    pos = mul(pos, worldMatrix);
    pos = mul(pos, viewMatrix);
    pos = mul(pos, projectionMatrix);
    return pos;
}

// Apply view and projection matrices
float4 ViewProject(float4 pos)
{
    pos = mul(pos, viewMatrix);
    pos = mul(pos, projectionMatrix);
    return pos;
}

// Apply projection matrix
float4 Project(float4 pos)
{
     return mul(pos, projectionMatrix);
}

// PCF filtering to find how much in shadow a pixel is
// As described by Frank Luna in Introduction to 3D GAME PROGRAMMING WITH DIRECTX 11, chapter 21
float CalculateShadowFactor(Texture2D depthMap, float4 worldPos, float bias, matrix lightViewMatrix, matrix lightProjectionMatrix)
{
    float4 lightViewPos = mul(worldPos, lightViewMatrix);
    lightViewPos = mul(lightViewPos, lightProjectionMatrix);
    
    // Calculate the projected texture coordinates.
    if (lightViewPos.w < 0) return 0; // prevent backwards projection
    float2 projTex = lightViewPos.xy / lightViewPos.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    
    // Test for depth data
    if (projTex.x < 0.f || projTex.x > 1.f || projTex.y < 0.f || projTex.y > 1.f)
    {
        //No depth data
        return 0;
    }
    
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPos.z / lightViewPos.w;
    lightDepthValue -= bias;
    
    float percentLit = 0.0f;
    float dx = 1.f / 1024.f; // Using 1024 x 1024 shadow maps
    
    // While Frank Luna uses a 3x3 kernel, I made this 5x5 one for greater smoothing
    const float2 offsets[25] = 
    {
        float2(-2 * dx, -2 * dx), float2(-dx, -2 * dx), float2(0.0f, -2 * dx), float2(dx, -2 * dx), float2(2 * dx, -2 * dx),
        float2(-2 * dx, -dx), float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx), float2(2 * dx, -dx),
        float2(-2 * dx, 0.0f), float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f), float2(2 * dx, 0.0f),
        float2(-2 * dx, +dx), float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx), float2(2 * dx, +dx),
        float2(-2 * dx, +2 * dx), float2(-dx, +2 * dx), float2(0.0f, +2 * dx), float2(dx, +2 * dx), float2(2 * dx, +2 * dx)
    };
    
    // 5x5 box filter pattern. Each sample does a 4-tap PCF.
    [unroll]
    for (int i = 0; i < 25; ++i)
    {
        percentLit += depthMap.SampleCmpLevelZero(shadowSampler,
        projTex + offsets[i], lightDepthValue).r;
    }
    
    // Average the samples.
    return percentLit /= 25.f;
}

// Calculate lighting from one light source
float4 CalculateLighting(inout float4 totalSpec, float3 lightDir, float3 normal, float4 diffuse, float4 specular, float specPow, float3 lightPos, float3 attenFactors, float spotConeAngle, float spotExponent, float3 fragmentPos, float3 camera)
{
    float3 lightVector = normalize(-lightDir);
    float spotlightIntensity = 1.f;
    float attenuation = 1.f;
    float fragmentAngle = -1.f;
    
    // Determine type of light and perform lighting calculations accordingly
    if ((lightDir.x == 0 && lightDir.y == 0 && lightDir.z == 0) || spotConeAngle > -1)
    {
        // Light is point or spotlight - find attenuation factor
        lightVector = lightPos - fragmentPos;
        float distance = length(lightVector);
        lightVector = normalize(lightVector);
        attenuation = 1 / (attenFactors.x + (attenFactors.y * distance) + (attenFactors.z * distance * distance) + 0.01f); // +0.01 to stop divide by zero
        
        if (lightDir.x == 0 && lightDir.y == 0 && lightDir.z == 0)
        {
            // Light is a point light, find basic lighting intensity and combine with diffuse
            diffuse *= saturate(dot(normal, lightVector));
        }
        else
        {
            // Light is a spotlight
            float cosMultiplier = (spotConeAngle > 0) ? (180 / spotConeAngle) : 0;
            fragmentAngle = acos(dot(-lightVector, normalize(lightDir)));
            if (degrees(fragmentAngle) <= spotConeAngle / 2) // Only light if fragment is within the cone of light
            {
                spotlightIntensity = pow(cos(fragmentAngle * cosMultiplier), spotExponent);
                diffuse *= spotlightIntensity;
            }
            else
                diffuse *= 0;
        }
    }
    else
    {
        // Light is a directional light
        diffuse *= saturate(dot(normal, lightVector));
    }
    
    // Increase total specular value
    if (specPow < 0 || (spotConeAngle > -1 && degrees(fragmentAngle) >= spotConeAngle / 2))
    {
        // Material isnt reflective or light is a spotlight and fragment is out of the light cone
        specular = 0.0f;
    }
    else
    {
        // Blinn-phong specular calculation       
        float3 viewVector = normalize(cameraPos - fragmentPos);
        float3 halfway = normalize(lightVector + viewVector);
        float specularIntensity = pow(max(dot(normal, halfway), 0.0), specPow) * spotlightIntensity;
        specular = specular * specularIntensity * attenuation;
        totalSpec += specular;
    }
    
    return diffuse * attenuation;
}

// Calculate lighting from all light sources
float4 CalculateFinalColour(float3 normal, float3 fragmentPos, float4 textureColour = float4(-1, -1, -1, -1))
{
    float shadowMapBias = 0.001f;
    float4 totalDiffuseColour = { 0, 0, 0, 0 };
    float4 totalSpecularColour = { 0, 0, 0, 0 };
    float shadowFactor = 0.f;
    
    // Lighting calculations for each light
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        if (lightDirection[i].x == 0 && lightDirection[i].y == 0 && lightDirection[i].z == 0)
        {
            // Light is a point light - lighting for each shadow map
            for (int j = 0; j < 6; j++)
            {
                shadowFactor = CalculateShadowFactor(depthMapTexture[i * 6 + j], float4(fragmentPos, 1), shadowBias, lightViews[i * 6 + j], lightProjection[i]);
                if (shadowFactor > 0.0f)
                {
                    // Add diffuse colour
                    totalDiffuseColour += CalculateLighting(totalSpecularColour, lightDirection[i], normal, lightDiffuse[i], lightSpecular[i], lightAngleExpSpecpow[i].z, lightPosition[i], lightAttenFac[i], lightAngleExpSpecpow[i].x, lightAngleExpSpecpow[i]
                    .y, fragmentPos, cameraPos) * shadowFactor;                       
                }              
            }
        }
        else
        {
            // Light is directional or a spot light
            shadowFactor = CalculateShadowFactor(depthMapTexture[i * 6], float4(fragmentPos, 1), shadowBias, lightViews[i * 6], lightProjection[i]);
            if (shadowFactor > 0.0f)
            {
                // Add diffuse colour
                totalDiffuseColour += CalculateLighting(totalSpecularColour, lightDirection[i], normal, lightDiffuse[i], lightSpecular[i], lightAngleExpSpecpow[i].z, lightPosition[i], lightAttenFac[i], lightAngleExpSpecpow[i].x, lightAngleExpSpecpow[i]
                    .y, fragmentPos, cameraPos) * shadowFactor;
            }
        }
        // Add this lights ambient colour
        totalDiffuseColour += lightAmbient[i];
    }
    
    // Combine and return the total diffuse, specular and (if provided) texture colour
    if (textureColour.x == -1)
    {
        // No texture provided, return only lighting
        return saturate(totalDiffuseColour + totalSpecularColour);
    }
    else
    {
        return saturate(saturate(totalDiffuseColour) * textureColour + totalSpecularColour);
    }
}