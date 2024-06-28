#pragma once
#include "DXF.h"
#include "Buffers.h"

using namespace std;
using namespace DirectX;

class MyLight;

// Draws terrain based on a heightmap, depth only
class TerrainDepthShader : public BaseShader
{
public:
	TerrainDepthShader(ID3D11Device* device, HWND hwnd, Buffers* buffs);
	~TerrainDepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* heightMap);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename);

	Buffers* buffers;
};
