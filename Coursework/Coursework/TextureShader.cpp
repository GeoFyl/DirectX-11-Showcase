#include "TextureShader.h"

TextureShader::TextureShader(ID3D11Device* device, HWND hwnd, Buffers* buffs) : BaseShader(device, hwnd)
{
	buffers = buffs;
	initShader(L"basic_vs.cso", L"texture_ps.cso");
}

TextureShader::~TextureShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the constant buffers
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (typeBuffer)
	{	
		typeBuffer->Release();
		typeBuffer = 0;
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


void TextureShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Buffer to indicate whether this is a shadowmap texture
	D3D11_BUFFER_DESC typeBufferDesc;
	typeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	typeBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	typeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	typeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	typeBufferDesc.MiscFlags = 0;
	typeBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&typeBufferDesc, NULL, &typeBuffer);
}


void TextureShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, bool isShadowMap)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	deviceContext->VSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());

	deviceContext->Map(typeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	XMFLOAT4* typePtr = (XMFLOAT4*)mappedResource.pData;
	typePtr->x = isShadowMap;
	deviceContext->Unmap(typeBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &typeBuffer);

	// Set shader texture and sampler resources 
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, buffers->getTextureSampler());
}





