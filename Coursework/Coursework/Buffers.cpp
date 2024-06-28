#include "Buffers.h"

// Create buffers
Buffers::Buffers(ID3D11Device* device, ID3D11DeviceContext* ctx)
{
	renderer = device;
	deviceContext = ctx;
	createSamplers();
	createMatrixBuffer();
	createLightsBuffer();
	createTessellationBuffer();
	createTerrainBuffer();
}

Buffers::~Buffers()
{
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (terrainBuffer)
	{
		terrainBuffer->Release();
		terrainBuffer = 0;
	}
	if (lightsBuffer)
	{
		lightsBuffer->Release();
		lightsBuffer = 0;
	}
	if (tessBuffer)
	{
		tessBuffer->Release();
		tessBuffer = 0;
	}
	if (sampleHeightMap)
	{
		sampleHeightMap->Release();
		sampleHeightMap = 0;
	}
	if (sampleShadows)
	{
		sampleShadows->Release();
		sampleShadows = 0;
	}
	if (sampleTexture)
	{
		sampleTexture->Release();
		sampleTexture = 0;
	}
}

// Update tessellation and terrain buffers
void Buffers::setTessTerrainBuffers(float* minMaxLodDist, bool distanceBased, float heightScale, float uvSteps)
{
	setTessellationBuffer(minMaxLodDist, distanceBased);
	setTerrainBuffer(heightScale, uvSteps);
}

// Send shadow maps to pixel shaders
void Buffers::setShadowMaps(std::unique_ptr<ShadowMap>* shadowMaps)
{
	ID3D11ShaderResourceView* shadowMap;
	for (int i = 0; i < NUM_LIGHTS * 6; i++) {
		shadowMap = shadowMaps[i]->getDepthMapSRV();
		deviceContext->PSSetShaderResources(5 + i, 1, &shadowMap);
	}
}

// Create sampler state descriptions
void Buffers::createSamplers()
{
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleHeightMap);

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	renderer->CreateSamplerState(&samplerDesc, &sampleTexture);

	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	renderer->CreateSamplerState(&samplerDesc, &sampleShadows);
}

void Buffers::createMatrixBuffer()
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);
}

void Buffers::setMatrixBuffer(const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix)
{
	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Send data
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBufferType* matPtr = (MatrixBufferType*)mappedResource.pData;
	matPtr->world = tworld;// worldMatrix;
	matPtr->view = tview;
	matPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
}

void Buffers::createLightsBuffer()
{
	D3D11_BUFFER_DESC lightsBufferDesc;
	lightsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightsBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightsBufferDesc.MiscFlags = 0;
	lightsBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightsBufferDesc, NULL, &lightsBuffer);
}

void Buffers::setLightsBuffer(std::unique_ptr<MyLight>* lights, float specPow, XMFLOAT3 cameraPos, float shadowBias, int debugMode)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(lightsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	LightBufferType* lightPtr = (LightBufferType*)mappedResource.pData;
	for (int i = 0; i < NUM_LIGHTS; i++) {
		lightPtr->ambient[i] = lights[i]->getAmbientColour();
		lightPtr->diffuse[i] = lights[i]->getDiffuseColour();
		lightPtr->specular[i] = lights[i]->getSpecularColour();
		XMFLOAT3 direction = lights[i]->getDirection();
		lightPtr->direction[i] = XMFLOAT4(direction.x, direction.y, direction.z, 1.f);
		XMFLOAT3 position = lights[i]->getPosition();
		lightPtr->position[i] = XMFLOAT4(position.x, position.y, position.z, 1.f);
		float* attenuation = lights[i]->attenuationFactors();
		lightPtr->attenuation[i] = XMFLOAT4(attenuation[0], attenuation[1], attenuation[2], 1.f);
		lightPtr->angleExponentSpecpow[i] = XMFLOAT4(*lights[i]->spotlightAngle(), *lights[i]->spotlightExponent(), specPow, 0);
		for (int j = 0; j < 6; j++) {
			lightPtr->views[i * 6 + j] = XMMatrixTranspose(*lights[i]->getView(j));
		}
		lightPtr->projection[i] = XMMatrixTranspose(*lights[i]->getProjMatrix()); // Get orthographic matrix if light is directional, else get projection matrix
	}
	lightPtr->debugMode = debugMode;
	lightPtr->cameraPos = cameraPos;
	lightPtr->shadowBias = shadowBias;
	deviceContext->Unmap(lightsBuffer, 0);
}

void Buffers::createTessellationBuffer()
{
	D3D11_BUFFER_DESC tessBufferDesc;
	tessBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessBufferDesc.ByteWidth = sizeof(TessellationBufferType);
	tessBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessBufferDesc.MiscFlags = 0;
	tessBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&tessBufferDesc, NULL, &tessBuffer);
}

void Buffers::setTessellationBuffer(float* minMaxLodDist, bool distanceBased)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(tessBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TessellationBufferType* tessPtr = (TessellationBufferType*)mappedResource.pData;
	tessPtr->minLOD = minMaxLodDist[0];
	tessPtr->maxLOD = minMaxLodDist[1];
	tessPtr->minDist = minMaxLodDist[2];
	tessPtr->maxDist = minMaxLodDist[3];
	tessPtr->distanceBased = distanceBased;
	tessPtr->padding = XMFLOAT3(0, 0, 0);
	deviceContext->Unmap(tessBuffer, 0);
}

void Buffers::createTerrainBuffer()
{
	D3D11_BUFFER_DESC terrainBufferDesc;
	terrainBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	terrainBufferDesc.ByteWidth = sizeof(TerrainBufferType);
	terrainBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	terrainBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	terrainBufferDesc.MiscFlags = 0;
	terrainBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&terrainBufferDesc, NULL, &terrainBuffer);
}

void Buffers::setTerrainBuffer(float heightScale, float uvSteps)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(terrainBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TerrainBufferType* terrPtr = (TerrainBufferType*)mappedResource.pData;
	terrPtr->heightScale = heightScale;
	terrPtr->uvSteps = uvSteps;
	terrPtr->padding = XMFLOAT2(0, 0);
	deviceContext->Unmap(terrainBuffer, 0);
}
