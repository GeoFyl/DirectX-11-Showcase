// Tessellation plane mesh.
#include "TessellationPlaneMesh.h"
#include <vector>

#define clamp(val, minimum, maximum) (max(min(val, maximum), minimum))

// initialise buffers and load texture.
TessellationPlaneMesh::TessellationPlaneMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution)
{
	resolution = lresolution;
	initBuffers(device);
}

// Release resources.
TessellationPlaneMesh::~TessellationPlaneMesh()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

// Build plane of control points
// General idea taken from Practical Rendering and Computation with Direct3D 11, pp. 419-420
void TessellationPlaneMesh::initBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	vertexCount = (resolution + 1) * (resolution + 1);
	indexCount = vertexCount * 12;

	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];
	index = 0;

	XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };

	std::vector<XMFLOAT2> uvs;
	for (j = 0; j <= resolution; j++)
	{
		for (i = 0; i <= resolution; i++)
		{
			// Load the vertex array with data.
			vertices[index].position = XMFLOAT3(i, 0.0f, j);
			vertices[index].texture = XMFLOAT2(((float)i / resolution), 1-((float)j / resolution));
			uvs.push_back(vertices[index].texture);
			vertices[index].normal = normal;

			index++;
		}
	}

	index = 0;
	for (j = 0; j < resolution; j++)
	{
		for (i = 0; i < resolution; i++)
		{
			//0-3: Main patch - preserve counter clockwise order for easy uv lerping in domain shader
			indices[index] = i + j * (resolution + 1);
			indices[index + 1] = (i + 1) + j * (resolution + 1);
			indices[index + 2] = (i + 1) + (j + 1) * (resolution + 1);
			indices[index + 3] = i + (j + 1) * (resolution + 1);
			//4-5: +x neighbour
			indices[index + 4] = clamp(i + 2, 0, resolution) + clamp(j, 0, resolution) * (resolution + 1);
			indices[index + 5] = clamp(i + 2, 0, resolution) + clamp(j + 1, 0, resolution) * (resolution + 1);
			//6-7: +z neighbour
			indices[index + 6] = clamp(i, 0, resolution) + clamp(j + 2, 0, resolution) * (resolution + 1);
			indices[index + 7] = clamp(i + 1, 0, resolution) + clamp(j + 2, 0, resolution) * (resolution + 1);
			//8-9: -x neighbour
			indices[index + 8] = clamp(i - 1, 0, resolution) + clamp(j, 0, resolution) * (resolution + 1);
			indices[index + 9] = clamp(i - 1, 0, resolution) + clamp(j + 1, 0, resolution) * (resolution + 1);
			//10-11: -z neighbour
			indices[index + 10] = clamp(i, 0, resolution) + clamp(j - 1, 0, resolution) * (resolution + 1);
			indices[index + 11] = clamp(i + 1, 0, resolution) + clamp(j - 1, 0, resolution) * (resolution + 1);

			index += 12;
		}
	}

	vertexBufferDesc = { sizeof(VertexType) * vertexCount, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	vertexData = { vertices, 0 , 0 };
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	indexBufferDesc = { sizeof(unsigned long) * indexCount, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
	indexData = { indices, 0, 0};
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	//delete[] indices;
	//indices = 0;
}

// Override sendData() to change topology type. Control point patch list is required for tessellation.
void TessellationPlaneMesh::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// Set the type of primitive that should be rendered from this vertex buffer, in this case control patch for tessellation.
	deviceContext->IASetPrimitiveTopology(top);
}

