#include "WaterShader.h"
#include "MyLight.h"

WaterShader::WaterShader(ID3D11Device* device, HWND hwnd, Buffers* buffs) : BaseShader(device, hwnd)
{
	buffers = buffs;
	initShader(L"water_vs.cso", L"tessellation_hs.cso", L"water_ds.cso", L"water_ps.cso");
}


WaterShader::~WaterShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (waterBuffer)
	{
		waterBuffer->Release();
		waterBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void WaterShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
	
	D3D11_BUFFER_DESC waterBufferDesc;
	waterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterBufferDesc.ByteWidth = sizeof(WaterBufferType);
	waterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBufferDesc.MiscFlags = 0;
	waterBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&waterBufferDesc, NULL, &waterBuffer);
}

void WaterShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}

void WaterShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* normalMap, float time, float speed, float* wave1, float* wave2, float* wave3, float* wave4)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	deviceContext->VSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());
	deviceContext->DSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());
	deviceContext->PSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());

	deviceContext->PSSetConstantBuffers(1, 1, buffers->getLightsBuffer());
	deviceContext->HSSetConstantBuffers(1, 1, buffers->getLightsBuffer());

	deviceContext->VSSetConstantBuffers(2, 1, buffers->getTessBuffer());
	deviceContext->HSSetConstantBuffers(2, 1, buffers->getTessBuffer());

	deviceContext->Map(waterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WaterBufferType* waterPtr = (WaterBufferType*)mappedResource.pData;
	waterPtr->time = time;
	waterPtr->speed = speed;
	waterPtr->wave1 = XMFLOAT4(wave1);
	waterPtr->wave2 = XMFLOAT4(wave2);
	waterPtr->wave3 = XMFLOAT4(wave3);
	waterPtr->wave4 = XMFLOAT4(wave4);
	waterPtr->padding = XMFLOAT2(0, 0);
	deviceContext->Unmap(waterBuffer, 0);
	deviceContext->VSSetConstantBuffers(3, 1, &waterBuffer);
	deviceContext->DSSetConstantBuffers(3, 1, &waterBuffer);
	deviceContext->PSSetConstantBuffers(3, 1, &waterBuffer);

	deviceContext->PSSetSamplers(0, 1, buffers->getTextureSampler());
	deviceContext->PSSetSamplers(2, 1, buffers->getShadowSampler());
	deviceContext->PSSetShaderResources(0, 1, &normalMap);
}
