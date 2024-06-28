// Tessellation Plane Mesh
#include "PlaneMesh.h"

using namespace DirectX;

// Plane of control points to be tessellated
class TessellationPlaneMesh : public BaseMesh
{

public:
	TessellationPlaneMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 100);
	~TessellationPlaneMesh();

	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST) override;

protected:
	void initBuffers(ID3D11Device* device) override;
	int resolution;
};