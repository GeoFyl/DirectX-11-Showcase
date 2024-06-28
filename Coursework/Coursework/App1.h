// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "WaterShader.h"
#include "TessellationPlaneMesh.h"
#include "TerrainShader.h"
#include "TerrainDepthShader.h"
#include "BasicShader.h"
#include "TextureShader.h"
#include "MyLight.h"
#include "DepthNormalBufferSSAO.h"
#include "BloomFilterBlurHor.h"
#include "BloomBlurVert.h"
#include "BloomCombine.h"
#include "GrassShader.h"
#include "GrassDepthShader.h"
#include "Buffers.h"
#include "ModelShader.h"
#include "ModelDepthShader.h"
#include <vector>


class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:

	void renderSceneDepth(XMMATRIX& world, XMMATRIX& view, XMMATRIX& projection);
	bool shadowDepthPass();

	//bool ssaoDepthPass();

	bool renderScenePass();

	void createBloomKernel();
	bool bloomPass();

	bool finalPass();
	bool render();
	void gui();

private:

	// Shader handlers
	std::unique_ptr<BasicShader> basicShader_;
	std::unique_ptr<TextureShader> textureShader_;
	std::unique_ptr<TerrainShader> terrainShader_;
	std::unique_ptr<TerrainDepthShader> terrainDepthShader_;
	std::unique_ptr<GrassShader> grassShader_;
	std::unique_ptr<GrassDepthShader> grassDepthShader_;
	std::unique_ptr<WaterShader> waterShader_;
	std::unique_ptr<ModelShader> modelShader_;
	std::unique_ptr<BloomFilterBlurHor> bloomBlurHShader_;
	std::unique_ptr<BloomBlurVert> bloomBlurVShader_;
	std::unique_ptr<BloomCombine> bloomCombineShader_;
	std::unique_ptr<ModelDepthShader> modelDepthShader_;

	// Meshes
	std::unique_ptr<OrthoMesh> screenOrthoMesh_;
	std::unique_ptr<OrthoMesh> shadowOrthoMesh_;
	std::unique_ptr<TessellationPlaneMesh> tessellatedPlane_;
	std::unique_ptr<AModel> tent_;
	std::unique_ptr<SphereMesh> sphere_;

	// Render targets
	std::unique_ptr<RenderTexture> renderTarget_;
	//DepthNormalBufferSSAO* ssao_buffer_;

	// Bloom
	bool bloomEnabled_ = true;
	float bloomKernel_[BLOOM_KERNEL_ARRAY_SIZE];
	float bloomEffect_ = 0.1f;
	float bloomThreshold_ = 0.5f;

	// Lighting
	std::unique_ptr<MyLight> lights_[NUM_LIGHTS];
	float pointLightPos_[3] = { 40, 10, 40 };
	float spotLightPos_[3] = { 5.68, 5.68, 16.67 };
	float dirLightDir_[3] = { 0, -1, 0.8 }; // Directional light direction
	float spotLightDir_[3] = { 0.2, -0.59, -1 }; // Spot light direction

	// Shadows
	// Each light can be configured as a point light so each has 6 shadow maps
	std::unique_ptr<ShadowMap> shadowMaps_[NUM_LIGHTS * 6];
	std::vector<ShadowMap*> debugShadowMaps_; // Shadow maps actually being drawn to, to be displayed for debugging 
	bool debugShadows_ = false;
	int debugShadowNumber_ = 0; // Which map is currently displayed for debugging 
	float shadowBias_ = 0.0084f;

	// Tessellation
	bool distanceBased_ = false;
	float minMaxLodDist[4] = { 2.5, 20, 1, 45 }; // min LOD, max LOD, min distance, max distance

	// Terrain
	float heightScale = 35;
	float uvSteps = 1;

	// Water
	float time_ = 0;
	float speed_ = 0.8f;
	// Wavelength, steepness, direction.x, direction.y
	float wave1_[4] = { 4.42f, 0.12f, 0.21f, 0.7f };
	float wave2_[4] = { 4.5f, 0.06f, -0.07f, 0.6f };
	float wave3_[4] = { 4.77f, 0.15f, -0.67f, 1.3f };
	float wave4_[4] = { 6.51f, 0.20f, -1.f, 1.f };

	// Misc
	std::unique_ptr<Buffers> buffers_; // object which stores buffers to be passed to shaders
	int debugMode_ = 0; // 0: none, 1: debug lighting, 2: debug normals
};

#endif