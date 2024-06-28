#include "BasicShader.h"

BasicShader::BasicShader(ID3D11Device* device, HWND hwnd, Buffers* buffs) : BaseShader(device, hwnd)
{
	buffers = buffs;
	initShader(L"basic_vs.cso", L"basic_ps.cso");
}

BasicShader::~BasicShader()
{
	// Release constant buffers.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (colourBuffer)
	{
		colourBuffer->Release();
		colourBuffer = 0;
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


void BasicShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{

	// Load (+ compile) shader files
	loadTextureVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Buffer to hold colour
	D3D11_BUFFER_DESC colourBufferDesc;
	colourBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	colourBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	colourBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	colourBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	colourBufferDesc.MiscFlags = 0;
	colourBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&colourBufferDesc, NULL, &colourBuffer);

}


void BasicShader::setShaderParameters(ID3D11DeviceContext* deviceContext, XMFLOAT4 colour)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	deviceContext->VSSetConstantBuffers(0, 1, buffers->getMatrixBuffer());

	deviceContext->Map(colourBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	XMFLOAT4* colourPtr = (XMFLOAT4*)mappedResource.pData;
	*colourPtr = colour;
	deviceContext->Unmap(colourBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &colourBuffer);

}





