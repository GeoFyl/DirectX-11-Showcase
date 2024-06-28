#pragma once

#include "BaseShader.h"
#include "Buffers.h"

using namespace std;
using namespace DirectX;

// Basic shader to display a texture
class TextureShader : public BaseShader
{
public:
	TextureShader(ID3D11Device* device, HWND hwnd, Buffers* buffs);
	~TextureShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, bool isShadowMap = false);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* typeBuffer;
	Buffers* buffers;
};

