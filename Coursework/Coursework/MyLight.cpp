#include "MyLight.h"
#include <math.h>

// Sets colour based on provided data
void MyLight::setColour(XMFLOAT4 colour)
{
    colour_[0] = colour.x;
    colour_[1] = colour.y;
    colour_[2] = colour.z;
    colour_[3] = colour.w;
    setColour();
}

//Sets colour based on internal data which may have been changed directly by the gui
void MyLight::setColour()
{
    float scale = colour_[3];
    setAmbientColour(colour_[0] * scale * 0.1f, colour_[1] * scale * 0.1f, colour_[2] * scale * 0.1f, 1.0f);
    setDiffuseColour(colour_[0] * scale, colour_[1] * scale, colour_[2] * scale, 1.0f);
    setSpecularColour(0.4f, 0.4f, 0.4f, 1.0f);
}

// Get the colour but clamped to full brightness, eg. (0.6, 0, 0) --> (1, 0, 0)
XMFLOAT4 MyLight::getColourMaxed()
{
    float maxComponent = fmaxf(fmaxf(colour_[0], colour_[1]), colour_[2]);
    float scalar = 1.f / maxComponent;
    return XMFLOAT4(colour_[0] * scalar, colour_[1] * scalar, colour_[2] * scalar, 1);
}


// Get orthographic matrix if light is directional, else get projection matrix
XMMATRIX* MyLight::getProjMatrix()
{
    if ((direction.x == 0 && direction.y == 0 && direction.z == 0) || spotAngle_ > -1)
    {
        // Light is point or spotlight
        return &projectionMatrix;
    }
    else {
        // Light is a directional light
        return &orthoMatrix;
    }
}

// Generate view matrices for a point light
void MyLight::generatePointViews()
{
    if (direction.x == 0 && direction.y == 0 && direction.z == 0) {
        // Light is a point light
        XMFLOAT3 directions[6] = {
            XMFLOAT3(1.f, 0.f, 0.f), // Right
            XMFLOAT3(-1.f, 0.f, 0.f), // Left
            XMFLOAT3(0.f, 1.f, 0.f), // Up
            XMFLOAT3(0.f, -1.f, 0.f), // Down
            XMFLOAT3(0.f, 0.f, 1.f), // Forward
            XMFLOAT3(0.f, 0.f, -1.f) // Back
        };

        //Generate view matrices
        for (int i = 0; i < 6; i++)
        {
            direction = directions[i];
            generateViewMatrix();
            viewMatrices_[i] = viewMatrix;
        }

        direction = XMFLOAT3(0, 0, 0);
    }
}

// Generate required view matrices 
void MyLight::generateViews()
{
    if (direction.x == 0 && direction.y == 0 && direction.z == 0) {
        // Light is a point light
        generatePointViews();
    }
    else generateViewMatrix();
    
}

// Get required view matrix
XMMATRIX* MyLight::getView(int i)
{
    if (direction.x == 0 && direction.y == 0 && direction.z == 0) {
        // Light is a point light
        return &viewMatrices_[i];
    }
    return &viewMatrix;
}

// Directional light needs a position for shadow mapping, so set direction and find resulting position
void MyLight::setDirectionAndFindPos(float x, float y, float z)
{
    setDirection(x, y, z);
    float magnitude = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    direction = XMFLOAT3(direction.x / magnitude, direction.y / magnitude, direction.z / magnitude);
    setPosition(50 - direction.x * 100, -direction.y * 100, 50 - direction.z * 100);
}
