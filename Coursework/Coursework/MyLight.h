#pragma once
#include "Light.h"

// Extension of the base light class
class MyLight : public Light {
public:
	float* colour() { return colour_; }
	void setColour(XMFLOAT4 colour); // Sets diffuse and ambient colours based on provided data
	void setColour(); // As above, based on internal data which may be changed directly by the gui
	XMFLOAT4 getColourMaxed(); // Get the colour but clamped to full brightness, eg. (0.6, 0, 0) --> (1, 0, 0)
	float* attenuationFactors() { return attenuation_; }
	float* spotlightAngle() { return &spotAngle_; }
	float* spotlightExponent() { return &exponent_; }
	XMMATRIX* getProjMatrix(); // Get orthographic matrix if light is directional, else get projection matrix
	void generateViews(); // Generate required view matrices 
	XMMATRIX* getView(int i = 0); // Get required view matrix
	void setDirectionAndFindPos(float x, float y, float z); // Directional light needs a position for shadow mapping, so set direction and find resulting position

protected:
	void generatePointViews(); // Generate view matrices for a point light

	float colour_[4];
	float attenuation_[3] = { 0.5, 0.125, 0.0 };
	float spotAngle_ = -1; // Spotlight cutoff angle
	float exponent_; // Exponent used in spotlight calculations
	XMMATRIX viewMatrices_[6];
};

