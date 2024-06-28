// tessellation shader.cpp
#include "GrassShader.h"
#include "MyLight.h"

GrassShader::GrassShader(ID3D11Device* device, HWND hwnd, Buffers* buffs) : BaseShader(device, hwnd)
{
	buffers = buffs;
	initShader(L"terrain_vs.cso", L"grass_tessellation_hs.cso", L"terrain_ds.cso", L"grass_gs.cso", L"grass_ps.cso");
}

GrassShader::~GrassShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void GrassShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Create density buffer
	D3D11_BUFFER_DESC densityBufferDesc;
	densityBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	densityBufferDesc.ByteWidth = sizeof(DensityBufferType);
	densityBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	densityBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	densityBufferDesc.MiscFlags = 0;
	densityBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&densityBufferDesc, NULL, &densityBuffer);
}

void GrassShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
	loadGeometryShader(gsFilename);
}


void GrassShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* heightMap)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	deviceContext->VSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());
	deviceContext->DSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());
	deviceContext->GSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());

	deviceContext->PSSetConstantBuffers(1, 1, buffers->getLightsBuffer());

	deviceContext->VSSetConstantBuffers(2, 1, buffers->getTessBuffer());

	deviceContext->VSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());
	deviceContext->DSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());
	deviceContext->GSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());
	deviceContext->PSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());

	result = deviceContext->Map(densityBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	DensityBufferType* densPtr = (DensityBufferType*)mappedResource.pData;
	densPtr->density = density;
	densPtr->padding = XMFLOAT3(0, 0, 0);
	deviceContext->Unmap(densityBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &densityBuffer);

	deviceContext->VSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->DSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->GSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->PSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->PSSetSamplers(2, 1, buffers->getShadowSampler()); //shadow
	deviceContext->PSSetSamplers(1, 1, buffers->getTextureSampler());
	deviceContext->VSSetShaderResources(0, 1, &heightMap);
	deviceContext->DSSetShaderResources(0, 1, &heightMap);
	deviceContext->GSSetShaderResources(0, 1, &heightMap);
	deviceContext->PSSetShaderResources(0, 1, &heightMap);
}


