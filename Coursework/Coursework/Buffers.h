#pragma once
#include "DXF.h"
#include "MyLight.h"

// Values and buffers used by many of the shaders

#define NUM_LIGHTS 4
#define BLOOM_KERNEL_ARRAY_SIZE 400

struct MatrixBufferType
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

struct LightBufferType
{
	XMFLOAT4 ambient[NUM_LIGHTS];
	XMFLOAT4 diffuse[NUM_LIGHTS];
	XMFLOAT4 specular[NUM_LIGHTS];
	XMFLOAT4 direction[NUM_LIGHTS];
	XMFLOAT4 position[NUM_LIGHTS];
	XMFLOAT4 attenuation[NUM_LIGHTS];
	XMFLOAT4 angleExponentSpecpow[NUM_LIGHTS];  // Spotlight angle, spotlight exponent, specular power
	XMMATRIX views[NUM_LIGHTS * 6];
	XMMATRIX projection[NUM_LIGHTS];
	XMFLOAT3 cameraPos;
	float debugMode;
	float shadowBias;
	XMFLOAT3 padding;
};

struct TessellationBufferType {
	float minLOD;
	float maxLOD;
	float minDist;
	float maxDist;
	float distanceBased;
	XMFLOAT3 padding;
};

struct TerrainBufferType {
	float heightScale;
	float uvSteps;
	XMFLOAT2 padding;
};

struct BloomBufferType {
	float kernel[BLOOM_KERNEL_ARRAY_SIZE];
	float radius;
	float threshold;
	XMFLOAT2 padding;
};

// Class holds buffer data commonly used among multiple shaders
class Buffers
{
public:
	Buffers(ID3D11Device* device, ID3D11DeviceContext* ctx);
	~Buffers();

	void setTessTerrainBuffers(float* minMaxLodDist, bool distanceBased, float heightScale, float uvSteps);
	void setMatrixBuffer(const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix);
	void setLightsBuffer(std::unique_ptr<MyLight>* lights, float specPow, XMFLOAT3 cameraPos, float shadowBias, int debugMode);
	void setTessellationBuffer(float* minMaxLodDist, bool distanceBased);
	void setTerrainBuffer(float heightScale, float uvSteps = 0);

	void setShadowMaps(std::unique_ptr<ShadowMap>* shadowMaps);

	ID3D11Buffer** getMatrixBuffer() { return &matrixBuffer; }
	ID3D11Buffer** getLightsBuffer() { return &lightsBuffer;	}
	ID3D11Buffer** getTessBuffer() { return &tessBuffer; }
	ID3D11Buffer** getTerrainBuffer() { return &terrainBuffer; }

	ID3D11SamplerState** getHeightMapSampler() { return &sampleHeightMap; }
	ID3D11SamplerState** getShadowSampler() { return &sampleShadows; }
	ID3D11SamplerState** getTextureSampler() { return &sampleTexture; }

protected:
	void createSamplers();
	void createMatrixBuffer();
	void createLightsBuffer();
	void createTessellationBuffer();
	void createTerrainBuffer();

	ID3D11DeviceContext* deviceContext;
	ID3D11Device* renderer;

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightsBuffer;
	ID3D11Buffer* tessBuffer;
	ID3D11Buffer* terrainBuffer;

	ID3D11SamplerState* sampleHeightMap;
	ID3D11SamplerState* sampleShadows;
	ID3D11SamplerState* sampleTexture;
};