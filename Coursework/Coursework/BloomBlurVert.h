#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

// Shader applies vertical blur
class BloomBlurVert : public BaseShader
{
public:
	BloomBlurVert(ID3D11Device* device, HWND hwnd, int w, int h);
	~BloomBlurVert();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1, int radius, float threshold, float* kernel);
	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };
	void unbind(ID3D11DeviceContext* dc);


private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

	ID3D11Buffer* bloomBuffer;

	// texture set
	ID3D11Texture2D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	int sWidth;
	int sHeight;

};