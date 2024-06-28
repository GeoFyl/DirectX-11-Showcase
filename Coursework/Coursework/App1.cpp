#include "App1.h"

App1::App1()
{

}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Using smart pointers - nothing else to do here
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"WaterNormalMap", L"res/water_textures_4k_normal.png"); // Source: https://www.cgtrader.com/products/water-texture-4k
	textureMgr->loadTexture(L"HeightMap", L"res/HeightMap.tif"); // Generated using Gaea: https://quadspinner.com/
	textureMgr->loadTexture(L"GrassTexture", L"res/grassTexture.png"); // Source: https://www.goodtextures.com/image/20024/grass-green-01-albedo
	textureMgr->loadTexture(L"GrassNormal", L"res/grassNormal.png"); // Source: https://www.goodtextures.com/image/20028/grass-green-01-normal
	textureMgr->loadTexture(L"MudTexture", L"res/mudTexture.jpg"); // Source: https://free-3dtextureshd.com/download/mud-soil-seamless-3d-textures-pbr-high-resolution-free-download-4k/
	textureMgr->loadTexture(L"MudNormal", L"res/mudNormal.jpg"); // Source: same as above ^
	textureMgr->loadTexture(L"TentTexture", L"res/bakedTex.png"); // Source: https://skfb.ly/oLFuS
	textureMgr->loadTexture(L"TentNormal", L"res/bakedNormals.png"); // Source: same as above ^

	// Initalise meshes
	shadowOrthoMesh_ = std::make_unique<OrthoMesh>(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenWidth / 4, screenWidth / 3, -screenHeight / 4); // Mesh to display shadow maps
	screenOrthoMesh_ = std::make_unique<OrthoMesh>(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight); // Mesh to display render target
	tessellatedPlane_ = std::make_unique<TessellationPlaneMesh>(renderer->getDevice(), renderer->getDeviceContext()); // Plane of control points to be tessellated
	sphere_ = std::make_unique<SphereMesh>(renderer->getDevice(), renderer->getDeviceContext());
	tent_ = std::make_unique<AModel>(renderer->getDevice(), "res/tent.obj"); // Source: https://skfb.ly/oLFuS

	// Initialize render targets
	renderTarget_ = std::make_unique<RenderTexture>(renderer->getDevice(), screenWidth, screenHeight, 0.1f, 100.f);
	//ssao_buffer_ = new DepthNormalBufferSSAO(renderer->getDevice(), screenWidth, screenHeight);

	// Initialise buffers object
	buffers_ = std::make_unique<Buffers>(renderer->getDevice(), renderer->getDeviceContext());

	// Initialise shader handlers
	basicShader_ = std::make_unique<BasicShader>(renderer->getDevice(), hwnd, buffers_.get());
	textureShader_ = std::make_unique<TextureShader>(renderer->getDevice(), hwnd, buffers_.get());
	terrainShader_ = std::make_unique<TerrainShader>(renderer->getDevice(), hwnd, buffers_.get());
	terrainDepthShader_ = std::make_unique<TerrainDepthShader>(renderer->getDevice(), hwnd, buffers_.get());
	grassShader_ = std::make_unique<GrassShader>(renderer->getDevice(), hwnd, buffers_.get());
	grassDepthShader_ = std::make_unique<GrassDepthShader>(renderer->getDevice(), hwnd, buffers_.get());
	waterShader_ = std::make_unique<WaterShader>(renderer->getDevice(), hwnd, buffers_.get());
	bloomBlurHShader_ = std::make_unique<BloomFilterBlurHor>(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	bloomBlurVShader_ = std::make_unique<BloomBlurVert>(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	bloomCombineShader_ = std::make_unique<BloomCombine>(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	modelShader_ = std::make_unique<ModelShader>(renderer->getDevice(), hwnd, buffers_.get());
	modelDepthShader_ = std::make_unique<ModelDepthShader>(renderer->getDevice(), hwnd, buffers_.get());

	// Variables for defining lights and shadow maps
	int shadowmapWidth = 1024;
	int shadowmapHeight = 1024;
	int sceneWidth = 142;
	int sceneHeight = 142;

	// Confirgure directional light
	lights_[0] = std::make_unique<MyLight>();
	lights_[0]->setColour(XMFLOAT4(0.7, 0.9, 1, 0.1));
	lights_[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 400.f);
	lights_[0]->setDirectionAndFindPos(dirLightDir_[0], dirLightDir_[1], dirLightDir_[2]);

	// Configure point light
	lights_[1] = std::make_unique<MyLight>();
	lights_[1]->setColour(XMFLOAT4(1, 1, 1, 0.8));
	lights_[1]->generateProjectionMatrix(2.f, 400.f);
	lights_[1]->setPosition(pointLightPos_[0], pointLightPos_[1], pointLightPos_[2]);

	// Configure spotlight
	lights_[2] = std::make_unique<MyLight>();
	lights_[2]->setColour(XMFLOAT4(1, 0.63, 0, 1));
	lights_[2]->setDirection(spotLightDir_[0], spotLightDir_[1], spotLightDir_[2]);
	lights_[2]->setPosition(spotLightPos_[0], spotLightPos_[1], spotLightPos_[2]);
	lights_[2]->generateProjectionMatrix(2.f, 400.f);
	*lights_[2]->spotlightAngle() = 45;
	*lights_[2]->spotlightExponent() = 1;

	// Configure tent point light
	lights_[3] = std::make_unique<MyLight>();
	lights_[3]->setColour(XMFLOAT4(1, 1, 1, 0.8));
	lights_[3]->generateProjectionMatrix(2.f, 400.f);
	lights_[3]->attenuationFactors()[1] = 0.3f;

	// Create shadow maps 
	for (int i = 0; i < NUM_LIGHTS; i++) {
		if (lights_[i]->getDirection().x == 0 && lights_[i]->getDirection().y == 0 && lights_[i]->getDirection().z == 0) {
			// Light is a point light
			for (int j = 0; j < 6; j++) {
				shadowMaps_[i * 6 + j] = std::make_unique<ShadowMap>(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
				debugShadowMaps_.push_back(shadowMaps_[i * 6 + j].get());
			}
		}
		else {
			for (int j = 0; j < 6; j++) {
				shadowMaps_[i * 6 + j] = std::make_unique<ShadowMap>(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
			}
			debugShadowMaps_.push_back(shadowMaps_[i * 6].get());
		}
	}

	// Create default kernel for gaussian blur
	createBloomKernel();
}

// Called every frame
bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Increase time by delta time since last frame
	time_ += timer->getTime();

	// Set position of tent point light
	lights_[3]->setPosition(7 + 5*cos(time_), 2.4, 8 + 5*sin(time_));

	// Generate the view matrix based on the camera's position.
	camera->update();

	buffers_.get()->setTessTerrainBuffers(minMaxLodDist, distanceBased_, heightScale, uvSteps);

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

// Render objects which will cast shadows
void App1::renderSceneDepth(XMMATRIX& world, XMMATRIX& view, XMMATRIX& projection)
{
	tessellatedPlane_->sendData(renderer->getDeviceContext()); // terrain and grass both use this mesh

	// Terrain
	terrainDepthShader_->setShaderParameters(renderer->getDeviceContext(), textureMgr->getTexture(L"HeightMap"));
	terrainDepthShader_->render(renderer->getDeviceContext(), tessellatedPlane_->getIndexCount());

	// Grass
	renderer->setBackfaceCulling(false);
	grassDepthShader_->setShaderParameters(renderer->getDeviceContext(), textureMgr->getTexture(L"HeightMap"));
	grassDepthShader_->render(renderer->getDeviceContext(), tessellatedPlane_->getIndexCount());

	// Tent
	world = DirectX::XMMatrixRotationY(0.7) * DirectX::XMMatrixScaling(0.08, 0.08, 0.08) * DirectX::XMMatrixTranslation(7, 0, 8);
	buffers_.get()->setMatrixBuffer(world, view, projection);
	tent_->sendData(renderer->getDeviceContext());
	modelDepthShader_->setShaderParameters(renderer->getDeviceContext());
	modelDepthShader_->render(renderer->getDeviceContext(), tent_->getIndexCount());
	renderer->setBackfaceCulling(true);
}

// For each light, render necessary shadow maps
bool App1::shadowDepthPass()
{
	XMMATRIX worldMatrix;
	XMMATRIX* lightProjectionMatrix;
	XMMATRIX* lightViewMatrix;
	for (int i = 0; i < NUM_LIGHTS; i++) {
		lightProjectionMatrix = lights_[i]->getProjMatrix();
		lights_[i]->generateViews();

		if (lights_[i]->getDirection().x == 0 && lights_[i]->getDirection().y == 0 && lights_[i]->getDirection().z == 0) {
			// Light is a point light
			for (int j = 0; j < 6; j++) { // Point lights have a shadow map for each direction
				worldMatrix = renderer->getWorldMatrix(); 
				shadowMaps_[i * 6 + j]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext()); // Set relevant shadow map as render target

				lightViewMatrix = lights_[i]->getView(j); // Get corresponding view matrix
				buffers_.get()->setMatrixBuffer(worldMatrix, *lightViewMatrix, *lightProjectionMatrix);
				renderSceneDepth(worldMatrix, *lightViewMatrix, *lightProjectionMatrix); // Render scene
			}
		}
		else {
			worldMatrix = renderer->getWorldMatrix();
			shadowMaps_[i * 6]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext()); // Set relevant shadow map as render target

			lightViewMatrix = lights_[i]->getView(); // Get the lights view matrix
			buffers_.get()->setMatrixBuffer(worldMatrix, *lightViewMatrix, *lightProjectionMatrix);
			renderSceneDepth(worldMatrix, *lightViewMatrix, *lightProjectionMatrix); // Render scene 
		}
	}
	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

	// Send shadow maps to pixel shader
	buffers_.get()->setShadowMaps(shadowMaps_);

	return true;
}

// Render entire scene to a texture, ready for post processing
bool App1::renderScenePass()
{
	XMFLOAT3 camPos = camera->getPosition();

	// Set render target
	renderTarget_->setRenderTarget(renderer->getDeviceContext());
	renderTarget_->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 1.0f);

	// Get the view, projection matrices from the camera and Direct3D objects.
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	buffers_.get()->setLightsBuffer(lights_, -1, camPos, shadowBias_, debugMode_);

	// Light spheres
	XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(pointLightPos_[0], pointLightPos_[1], pointLightPos_[2]);
	buffers_.get()->setMatrixBuffer(worldMatrix, viewMatrix, projectionMatrix);
	sphere_->sendData(renderer->getDeviceContext());
	basicShader_->setShaderParameters(renderer->getDeviceContext(), lights_[1]->getColourMaxed());
	basicShader_->render(renderer->getDeviceContext(), sphere_->getIndexCount());

	worldMatrix = DirectX::XMMatrixTranslation(spotLightPos_[0], spotLightPos_[1], spotLightPos_[2]);
	buffers_.get()->setMatrixBuffer(worldMatrix, viewMatrix, projectionMatrix);
	basicShader_->setShaderParameters(renderer->getDeviceContext(), lights_[2]->getColourMaxed());
	basicShader_->render(renderer->getDeviceContext(), sphere_->getIndexCount());

	XMFLOAT3 tentLightPos = lights_[3]->getPosition();
	worldMatrix = DirectX::XMMatrixScaling(0.2, 0.2, 0.2) * DirectX::XMMatrixTranslation(tentLightPos.x, tentLightPos.y, tentLightPos.z);
	buffers_.get()->setMatrixBuffer(worldMatrix, viewMatrix, projectionMatrix);
	basicShader_->setShaderParameters(renderer->getDeviceContext(), XMFLOAT4(1, 1, 1, 1));
	basicShader_->render(renderer->getDeviceContext(), sphere_->getIndexCount());

	// Same mesh is used by terrain, water and grass
	tessellatedPlane_->sendData(renderer->getDeviceContext());

	// Terrain
	worldMatrix = renderer->getWorldMatrix();
	buffers_.get()->setMatrixBuffer(worldMatrix, viewMatrix, projectionMatrix);
	terrainShader_->setShaderParameters(renderer->getDeviceContext(), textureMgr->getTexture(L"HeightMap"), textureMgr->getTexture(L"GrassTexture"), textureMgr->getTexture(L"GrassNormal"), textureMgr->getTexture(L"MudTexture"), textureMgr->getTexture(L"MudNormal"));
	terrainShader_->render(renderer->getDeviceContext(), tessellatedPlane_->getIndexCount());

	// Water
	if(debugMode_ != 2) renderer->setAlphaBlending(true);
	buffers_.get()->setLightsBuffer(lights_, 200, camPos, shadowBias_, debugMode_);
	worldMatrix = DirectX::XMMatrixRotationY(1.2) * DirectX::XMMatrixScaling(0.5, 1, 0.5) * DirectX::XMMatrixTranslation(8, 4.6 * (heightScale / 35), 61);
	buffers_.get()->setMatrixBuffer(worldMatrix, viewMatrix, projectionMatrix);
	waterShader_->setShaderParameters(renderer->getDeviceContext(), textureMgr->getTexture(L"WaterNormalMap"), time_, speed_, wave1_, wave2_, wave3_, wave4_);
	waterShader_->render(renderer->getDeviceContext(), tessellatedPlane_->getIndexCount());
	renderer->setAlphaBlending(false);

	// Grass
	buffers_.get()->setLightsBuffer(lights_, 20, camPos, shadowBias_, debugMode_);
	worldMatrix = renderer->getWorldMatrix();
	buffers_.get()->setMatrixBuffer(worldMatrix, viewMatrix, projectionMatrix);
	renderer->setBackfaceCulling(false); // Grass and tent both need backface culling disabled
	grassShader_->setShaderParameters(renderer->getDeviceContext(), textureMgr->getTexture(L"HeightMap"));
	grassShader_->render(renderer->getDeviceContext(), tessellatedPlane_->getIndexCount());

	// Tent
	buffers_.get()->setLightsBuffer(lights_, -1, camPos, shadowBias_, debugMode_);
	worldMatrix = DirectX::XMMatrixRotationY(0.7) * DirectX::XMMatrixScaling(0.08, 0.08, 0.08) * DirectX::XMMatrixTranslation(7, 0, 8);
	buffers_.get()->setMatrixBuffer(worldMatrix, viewMatrix, projectionMatrix);
	tent_->sendData(renderer->getDeviceContext());
	modelShader_->setShaderParameters(renderer->getDeviceContext(), textureMgr->getTexture(L"TentTexture"), textureMgr->getTexture(L"TentNormal"));
	modelShader_->render(renderer->getDeviceContext(), tent_->getIndexCount());
	renderer->setBackfaceCulling(true);

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

	return true;
}

// Generate a 1D gaussian kernel. Called whenever gui changes bloom effect
// Based on explanation from: https://dsp.stackexchange.com/a/54381
void App1::createBloomKernel()
{
	int n = bloomEffect_ * (BLOOM_KERNEL_ARRAY_SIZE - 1) + 1;
	float sigma = 0.3 * ((n-1) * 0.5-1) + 0.8;
	float sum = 0;

	for (int i = 0; i < n; i++) {
		bloomKernel_[i] = exp(-pow((i-(n-1) / 2),2) / (2*sigma*sigma));
		sum += bloomKernel_[i];
	}
	for (int i = 0; i < n; i++) {
		bloomKernel_[i] /= sum;
	}
}

// Apply bloom post processing effect 
bool App1::bloomPass()
{
	// Horiontal pass using unblurred copy of the scene, keeps only bright areas by filtering out dark pixels (where brightness < bloomThreshold_)
	bloomBlurHShader_->setShaderParameters(renderer->getDeviceContext(), renderTarget_->getShaderResourceView(), bloomEffect_*(BLOOM_KERNEL_ARRAY_SIZE-1)/2, bloomThreshold_, bloomKernel_);
	bloomBlurHShader_->compute(renderer->getDeviceContext(), ceil(sWidth / 256.f), sHeight, 1);
	bloomBlurHShader_->unbind(renderer->getDeviceContext());

	// Vertical blur using the horizontal blur result
	bloomBlurVShader_->setShaderParameters(renderer->getDeviceContext(), bloomBlurHShader_->getSRV(), bloomEffect_ * (BLOOM_KERNEL_ARRAY_SIZE - 1) / 2, bloomThreshold_, bloomKernel_);
	bloomBlurVShader_->compute(renderer->getDeviceContext(), sWidth, ceil((float)sHeight / 256.f), 1);
	bloomBlurVShader_->unbind(renderer->getDeviceContext());

	// Combine blurred result with original unblurred scene
	bloomCombineShader_->setShaderParameters(renderer->getDeviceContext(), renderTarget_->getShaderResourceView(), bloomBlurVShader_->getSRV());
	bloomCombineShader_->compute(renderer->getDeviceContext(), ceil((float)sWidth / 32.f), ceil((float)sHeight / 32.f), 1); // 32^2 = 1024, the max allowed amount of threads per group 
	bloomCombineShader_->unbind(renderer->getDeviceContext());

	return true;
}

// Render final scene to screen sized quad, then render UI
bool App1::finalPass()
{
	// Clear the scene
	renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);

	renderer->setWireframeMode(false); // Don't want to render the orthomeshes in wireframe mode

	renderer->setZBuffer(false);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	buffers_.get()->setMatrixBuffer(worldMatrix, orthoViewMatrix, orthoMatrix);
	
	// Render scene to screen quad
	screenOrthoMesh_->sendData(renderer->getDeviceContext());
	textureShader_->setShaderParameters(renderer->getDeviceContext(), (bloomEnabled_ && debugMode_ != 2) ? bloomCombineShader_->getSRV() : renderTarget_->getShaderResourceView());
	textureShader_->render(renderer->getDeviceContext(), screenOrthoMesh_->getIndexCount());

	//Debug shadow map view
	if (debugShadows_) {
		shadowOrthoMesh_->sendData(renderer->getDeviceContext());
		textureShader_->setShaderParameters(renderer->getDeviceContext(), debugShadowMaps_[debugShadowNumber_]->getDepthMapSRV(), true);
		textureShader_->render(renderer->getDeviceContext(), shadowOrthoMesh_->getIndexCount());
	}

	renderer->setZBuffer(true);

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

// Call passes in order
bool App1::render()
{
	shadowDepthPass();
	//ssaoDepthPass();
	renderScenePass();
	if (bloomEnabled_ && debugMode_ != 2) bloomPass(); // don't apply bloom if not enabled and/or debugging normals
	finalPass();
	return true;
}

// Draw UI and call some functions when necessary
void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	if (ImGui::CollapsingHeader("Lighting")) {
		ImGui::Text("--------Directional light--------");
		ImGui::Text("Direction:");
		ImGui::PushID(0);
		ImGui::PushItemWidth(100);
		// Set direction and position of directional light
		if(ImGui::SliderFloat("x", dirLightDir_, -1, 1)) lights_[0]->setDirectionAndFindPos(dirLightDir_[0], dirLightDir_[1], dirLightDir_[2]); ImGui::SameLine();
		if (ImGui::SliderFloat("y", &dirLightDir_[1], -1, 0)) lights_[0]->setDirectionAndFindPos(dirLightDir_[0], dirLightDir_[1], dirLightDir_[2]); ImGui::SameLine();
		if (ImGui::SliderFloat("z", &dirLightDir_[2], -1, 1)) lights_[0]->setDirectionAndFindPos(dirLightDir_[0], dirLightDir_[1], dirLightDir_[2]);

		ImGui::PopItemWidth();
		if (ImGui::ColorEdit4("Colour", lights_[0]->colour(), ImGuiColorEditFlags_AlphaBar)) lights_[0]->setColour();
		ImGui::PopID();
		ImGui::PushID(1);
		ImGui::Text("-----------Point light-----------");
		// Set position of point light
		if(ImGui::SliderFloat3("Position", pointLightPos_, 0, 100)) lights_[1]->setPosition(pointLightPos_[0], pointLightPos_[1], pointLightPos_[2]);
		ImGui::SliderFloat3("Attenuation CLQ", lights_[1]->attenuationFactors(), 0, 1);
		if (ImGui::ColorEdit4("Colour", lights_[1]->colour(), ImGuiColorEditFlags_AlphaBar)) lights_[1]->setColour();
		ImGui::PopID();
		ImGui::PushID(2);
		ImGui::Text("-----------Spot light------------");
		// Set direction and position of spot light
		if(ImGui::SliderFloat3("Position", spotLightPos_, 0, 100)) lights_[2]->setPosition(spotLightPos_[0], spotLightPos_[1], spotLightPos_[2]);
		if(ImGui::SliderFloat3("Direction", spotLightDir_, -1, 1)) lights_[2]->setDirection(spotLightDir_[0], spotLightDir_[1], spotLightDir_[2]);
		ImGui::SliderFloat("Angle", lights_[2]->spotlightAngle(), 0, 90);
		ImGui::SliderFloat("Exponent", lights_[2]->spotlightExponent(), 0, 10);
		ImGui::SliderFloat3("Attenuation CLQ", lights_[2]->attenuationFactors(), 0, 1);
		if (ImGui::ColorEdit4("Colour", lights_[2]->colour(), ImGuiColorEditFlags_AlphaBar)) lights_[2]->setColour();
		ImGui::PopID();
		ImGui::Text("---------------------------------");
		ImGui::RadioButton("No debugging", &debugMode_, 0); ImGui::SameLine();
		ImGui::RadioButton("Debug lighting", &debugMode_, 1); ImGui::SameLine();
		ImGui::RadioButton("Debug normals", &debugMode_, 2);
	}
	if (ImGui::CollapsingHeader("Bloom")) {
		ImGui::Checkbox("Enable bloom", &bloomEnabled_);
		if (ImGui::SliderFloat("Bloom Effect", &bloomEffect_, 0.01, 1)) {
			createBloomKernel();
		}
		ImGui::SliderFloat("Threshold", &bloomThreshold_, 0, 1);
	}
	if (ImGui::CollapsingHeader("Shadows")) {
		ImGui::SliderFloat("Shadow bias", &shadowBias_, 0.0f, 0.05f, "%.4f");
		ImGui::Checkbox("Debug shadow maps", &debugShadows_);
		ImGui::Text("View shadow map: "); ImGui::SameLine();
		if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { debugShadowNumber_ = max(debugShadowNumber_ - 1, 0); } ImGui::SameLine();
		ImGui::Text("%d", debugShadowNumber_); ImGui::SameLine();
		if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { debugShadowNumber_ = min(debugShadowNumber_ + 1, debugShadowMaps_.size() - 1); }

	}
	if (ImGui::CollapsingHeader("Tessellation")) {
		ImGui::RadioButton("Based on distance", (int*)&distanceBased_, 1); ImGui::SameLine();
		ImGui::RadioButton("Based on screen area", (int*)&distanceBased_, 0);
		ImGui::SliderFloat("Min LOD", &minMaxLodDist[0], 1, 64);
		ImGui::SliderFloat("Max LOD", &minMaxLodDist[1], 1, 64);
		ImGui::SliderFloat("Min Distance", &minMaxLodDist[2], 1, 20);
		ImGui::SliderFloat("Max Distance", &minMaxLodDist[3], 20, 100);
	}
	if (ImGui::CollapsingHeader("Terrain")) {
		ImGui::SliderFloat("Terrain Height", &heightScale, 10, 70);
		if (ImGui::SliderFloat("Grass Density", grassShader_->getDensity(), 1, 64)) {
			*grassDepthShader_->getDensity() = *grassShader_->getDensity();
		}
	}
	if (ImGui::CollapsingHeader("Water")) {
		ImGui::SliderFloat("Speed", &speed_, 0, 10);
		ImGui::Text("-----------------------------------------------");
		ImGui::Text("Wavelength, Steepness, Direction.x, Direction.y");
		ImGui::Text("Wave 1:");
		ImGui::PushItemWidth(100);
		ImGui::SliderFloat("W1", wave1_, 0, 10); ImGui::SameLine();
		ImGui::SliderFloat("S1", &wave1_[1], 0, 1); ImGui::SameLine();
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(200);
		ImGui::SliderFloat2("D1", &wave1_[2], -1, 1);
		ImGui::PopItemWidth();
		ImGui::Text("Wave 2:");
		ImGui::PushItemWidth(100);
		ImGui::SliderFloat("W2", wave2_, 0, 10); ImGui::SameLine();
		ImGui::SliderFloat("S2", &wave2_[1], 0, 1); ImGui::SameLine();
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(200);
		ImGui::SliderFloat2("D2", &wave2_[2], -1, 1);
		ImGui::PopItemWidth();
		ImGui::Text("Wave 3:");
		ImGui::PushItemWidth(100);
		ImGui::SliderFloat("W3", wave3_, 0, 10); ImGui::SameLine();
		ImGui::SliderFloat("S3", &wave3_[1], 0, 1); ImGui::SameLine();
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(200);
		ImGui::SliderFloat2("D3", &wave3_[2], -1, 1);
		ImGui::PopItemWidth();
		ImGui::Text("Wave 4:");
		ImGui::PushItemWidth(100);
		ImGui::SliderFloat("W4", wave4_, 0, 10); ImGui::SameLine();
		ImGui::SliderFloat("S4", &wave4_[1], 0, 1); ImGui::SameLine();
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(200);
		ImGui::SliderFloat2("D4", &wave4_[2], -1, 1);
		ImGui::PopItemWidth();
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


// Had started implementing SSAO but ran out of time :(

//bool App1::ssaoDepthPass()
//{
//	// Set the render target to be the buffer for holding normals and depths
//	ssao_buffer_->setRenderTarget(renderer->getDeviceContext());
//	ssao_buffer_->clearRenderTarget(renderer->getDeviceContext());
//
//	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
//	XMMATRIX worldMatrix = renderer->getWorldMatrix();
//	XMMATRIX viewMatrix = camera->getViewMatrix();
//	XMMATRIX projectionMatrix = *lights_[0]->getProjMatrix(); // TODO: add buffers own projection matrix
//
//	// Terrain
//	tessellatedPlane_->sendData(renderer->getDeviceContext());
//	terrainShader_->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, lights_, textureMgr->getTexture(L"HeightMap"), textureMgr->getTexture(L"GrassTexture"), textureMgr->getTexture(L"GrassNormal"), textureMgr->getTexture(L"MudTexture"), textureMgr->getTexture(L"MudNormal"), shadowMaps_, shadowBias_, camera->getPosition(), minMaxLodDist, distanceBased_, heightScale, uvSteps, 2);
//	terrainShader_->render(renderer->getDeviceContext(), tessellatedPlane_->getIndexCount());
//
//	// Reset the render target back to the original back buffer 
//	renderer->setBackBufferRenderTarget();
//
//	renderer->getDeviceContext()->GenerateMips(ssao_buffer_->getShaderResourceView());
//
//	return true;
//}