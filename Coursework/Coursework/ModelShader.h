#pragma once

#include "BaseShader.h"
#include "Buffers.h"

using namespace std;
using namespace DirectX;

class MyLight;

// Shader for drawing models
class ModelShader : public BaseShader
{
public:
	ModelShader(ID3D11Device* device, HWND hwnd, Buffers* buffs);
	~ModelShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalMap);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	Buffers* buffers;
};

