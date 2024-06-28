// tessellation shader.cpp
#include "TerrainDepthShader.h"
#include "MyLight.h"

TerrainDepthShader::TerrainDepthShader(ID3D11Device* device, HWND hwnd, Buffers* buffs) : BaseShader(device, hwnd)
{
	buffers = buffs;
	initShader(L"terrain_vs.cso", L"tessellation_hs.cso", L"terrain_ds.cso");
}

TerrainDepthShader::~TerrainDepthShader()
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

void TerrainDepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
}

void TerrainDepthShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, L" ");

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void TerrainDepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* heightMap)
{
	
	deviceContext->VSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());
	deviceContext->DSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());

	deviceContext->HSSetConstantBuffers(1, 1, buffers->getLightsBuffer());

	deviceContext->VSSetConstantBuffers(2, 1, buffers->getTessBuffer());
	deviceContext->HSSetConstantBuffers(2, 1, buffers->getTessBuffer());

	deviceContext->VSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());
	deviceContext->DSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());

	deviceContext->VSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->DSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->VSSetShaderResources(0, 1, &heightMap);
	deviceContext->DSSetShaderResources(0, 1, &heightMap);

}


