#pragma once
#include "DXF.h"
#include "Buffers.h"

using namespace std;
using namespace DirectX;

class MyLight;

// Draws grass using geometry shader, depth only
class GrassDepthShader : public BaseShader
{
public:
	GrassDepthShader(ID3D11Device* device, HWND hwnd, Buffers* buffs);
	~GrassDepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* heightMap);
	float* getDensity() { return &density; }
private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename);

private:
	struct DensityBufferType {
		float density;
		XMFLOAT3 padding;
	};

	float density = 5;

	ID3D11Buffer* densityBuffer;

	Buffers* buffers;
};
