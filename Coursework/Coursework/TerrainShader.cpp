// tessellation shader.cpp
#include "TerrainShader.h"
#include "MyLight.h"

TerrainShader::TerrainShader(ID3D11Device* device, HWND hwnd, Buffers* buffs) : BaseShader(device, hwnd)
{
	buffers = buffs;
	initShader(L"terrain_vs.cso", L"tessellation_hs.cso", L"terrain_ds.cso", L"terrain_ps.cso");
}

TerrainShader::~TerrainShader()
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

void TerrainShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
}

void TerrainShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}

void TerrainShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* heightMap, ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* grassNormal, ID3D11ShaderResourceView* mudTexture, ID3D11ShaderResourceView* mudNormal)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	deviceContext->VSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());
	deviceContext->DSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());

	deviceContext->HSSetConstantBuffers(1, 1, buffers->getLightsBuffer());
	deviceContext->PSSetConstantBuffers(1, 1, buffers->getLightsBuffer());

	deviceContext->VSSetConstantBuffers(2, 1, buffers->getTessBuffer());
	deviceContext->HSSetConstantBuffers(2, 1, buffers->getTessBuffer());

	deviceContext->VSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());
	deviceContext->DSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());
	deviceContext->PSSetConstantBuffers(3, 1, buffers->getTerrainBuffer());

	deviceContext->VSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->DSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->PSSetSamplers(0, 1, buffers->getHeightMapSampler());
	deviceContext->PSSetSamplers(2, 1,buffers->getShadowSampler());  //shadow
	deviceContext->PSSetSamplers(1, 1,buffers->getTextureSampler());
	deviceContext->VSSetShaderResources(0, 1, &heightMap);
	deviceContext->DSSetShaderResources(0, 1, &heightMap);
	deviceContext->PSSetShaderResources(0, 1, &heightMap);
	deviceContext->PSSetShaderResources(1, 1, &grassTexture);
	deviceContext->PSSetShaderResources(2, 1, &grassNormal);
	deviceContext->PSSetShaderResources(3, 1, &mudTexture);
	deviceContext->PSSetShaderResources(4, 1, &mudNormal);
}


