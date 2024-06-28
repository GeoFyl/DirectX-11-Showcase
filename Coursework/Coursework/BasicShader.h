#pragma once

#include "BaseShader.h"
#include "Buffers.h"

using namespace std;
using namespace DirectX;

// Basic shader to display a colour
class BasicShader : public BaseShader
{
public:
	BasicShader(ID3D11Device* device, HWND hwnd, Buffers* buffs);
	~BasicShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, XMFLOAT4 colour);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	Buffers* buffers;
	ID3D11Buffer* colourBuffer;
};

