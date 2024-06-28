#pragma once
#include <d3d11.h>

// Modified RenderTexture class for holding depth and normal information with mipmaps allowed
class DepthNormalBufferSSAO {
public:
	DepthNormalBufferSSAO(ID3D11Device* device, int textureWidth, int textureHeight);

	void setRenderTarget(ID3D11DeviceContext* deviceContext);		///< Set this render texture as the render target
	void clearRenderTarget(ID3D11DeviceContext* deviceContext);	///< Empties the render texture, provide device context and RGBA (background colour)
	ID3D11ShaderResourceView* getShaderResourceView() { return shaderResourceView; }		///< Get the data from this render target as a texture resource.

	int getTextureWidth() { return textureWidth; }		///< Get width of this render texture
	int getTextureHeight() { return textureHeight; }	///< Get height of this render texture

protected:
	int textureWidth, textureHeight;
	ID3D11Texture2D* renderTargetTexture;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11ShaderResourceView* shaderResourceView;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11DepthStencilView* depthStencilView;
};

