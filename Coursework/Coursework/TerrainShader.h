#pragma once
#include "DXF.h"
#include "Buffers.h"

using namespace std;
using namespace DirectX;

class MyLight;

// Draws terrain based on a heightmap
class TerrainShader : public BaseShader
{
public:
	TerrainShader(ID3D11Device* device, HWND hwnd, Buffers* buffs);
	~TerrainShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* heightMap, ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* grassNormal, ID3D11ShaderResourceView* mudTexture, ID3D11ShaderResourceView* mudNormal);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

	Buffers* buffers;
};
