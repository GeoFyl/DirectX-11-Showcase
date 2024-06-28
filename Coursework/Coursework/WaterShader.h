#pragma once
#include "DXF.h"
#include "Buffers.h"

class MyLight;

using namespace DirectX;

// Draws water with waves calculated in real-time 
class WaterShader : public BaseShader {
private:
	struct WaterBufferType {
		XMFLOAT4 wave1;
		XMFLOAT4 wave2;
		XMFLOAT4 wave3;
		XMFLOAT4 wave4;
		float time;
		float speed;
		XMFLOAT2 padding;
	};

public:
	WaterShader(ID3D11Device* device, HWND hwnd, Buffers* buffs);
	~WaterShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* normalMap, float time, float speed, float* wave1, float* wave2, float* wave3, float* wave4);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

	ID3D11Buffer* waterBuffer;
	Buffers* buffers;
};

