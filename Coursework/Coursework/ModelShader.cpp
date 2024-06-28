#include "ModelShader.h"
#include "MyLight.h"

ModelShader::ModelShader(ID3D11Device* device, HWND hwnd, Buffers* buffs) : BaseShader(device, hwnd)
{
	buffers = buffs;
	initShader(L"model_vs.cso", L"model_ps.cso");
}

ModelShader::~ModelShader()
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

	//Release base shader components
	BaseShader::~BaseShader();
}


void ModelShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
}


void ModelShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalMap)
{
	deviceContext->VSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());

	deviceContext->PSSetConstantBuffers(1, 1, buffers->getLightsBuffer());

	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &normalMap);
	deviceContext->PSSetSamplers(0, 1, buffers->getTextureSampler());
}





