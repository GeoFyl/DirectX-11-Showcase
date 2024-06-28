#pragma once

#include "BaseShader.h"
#include "Buffers.h"

using namespace std;
using namespace DirectX;

class MyLight;

// Shader for drawing models, depth only
class ModelDepthShader : public BaseShader
{
public:
	ModelDepthShader(ID3D11Device* device, HWND hwnd, Buffers* buffs);
	~ModelDepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	Buffers* buffers;
};

