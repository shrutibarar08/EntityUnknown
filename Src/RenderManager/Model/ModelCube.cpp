#include "ModelCube.h"

bool ModelCube::Build(ID3D11Device* device)
{
	if (m_Initialized)
	{
		return true;
	}
	//~ Build Vertices
	BuildVertex();
	BuildIndex();

	m_SharedCubeBuffer = std::make_shared<CubeBuffer>(
		m_Vertices,
		m_Indices);

	m_CubeBuffer = std::make_unique<ModelInstance<CubeBuffer>>(m_SharedCubeBuffer);
	m_CubeBuffer->Init(device);

	//~ Build Shaders
	m_ShaderResources = std::make_unique<ShaderResource>();
	m_ShaderResources->AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources->AddElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources->AddElement("COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = L"Shader/Shape/CubeShaderVS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources->SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = L"Shader/Shape/CubeShaderPS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources->SetPixelShaderPath(vertexDesc);

	//~ Vertex Constant Buffer
	m_VertexConstantBuffer = std::make_unique<ConstantBuffer<WORLD_TRANSFORM>>(device);

	if (!m_ShaderResources->Build(device))
	{
		return false;
	}
	m_Initialized = true;
	return true;
}

bool ModelCube::Render(ID3D11DeviceContext* deviceContext) const
{
	if (!m_Initialized) return false;

	m_CubeBuffer->Render(deviceContext);
	m_VertexConstantBuffer->Update(deviceContext, &m_WorldTransform);
	deviceContext->VSSetConstantBuffers(
		0u,
		1u,
		m_VertexConstantBuffer->GetAddressOf()
	);
	m_ShaderResources->Render(deviceContext);

	deviceContext->DrawIndexed(
		m_CubeBuffer->GetIndexCount(),
		0u,
		0u
	);
	return true;
}

void ModelCube::BuildVertex()
{
	std::vector<CUBE_VERTEX_DESC> vertices =
	{
		{{-1, -1, -1}, {0, 0, -1}, {1, 0, 0, 1}},  // Red
		{{ 1, -1, -1}, {0, 0, -1}, {0, 1, 0, 1}},  // Green
		{{ 1,  1, -1}, {0, 0, -1}, {0, 0, 1, 1}},  // Blue
		{{-1,  1, -1}, {0, 0, -1}, {1, 1, 0, 1}},  // Yellow
		{{-1, -1,  1}, {0, 0,  1}, {1, 0, 1, 1}},  // Magenta
		{{ 1, -1,  1}, {0, 0,  1}, {0, 1, 1, 1}},  // Cyan
		{{ 1,  1,  1}, {0, 0,  1}, {1, 1, 1, 1}},  // White
		{{-1,  1,  1}, {0, 0,  1}, {0, 0, 0, 1}},  // Black
	};

	for (int i = 0; i < 8; ++i)
	{
		m_Vertices[i] = vertices[i];
	}
}

void ModelCube::BuildIndex()
{
	std::vector<uint32_t> indices =
	{
		0,1,2, 0,2,3,  // front
		4,6,5, 4,7,6,  // back
		4,5,1, 4,1,0,  // bottom
		3,2,6, 3,6,7,  // top
		1,5,6, 1,6,2,  // right
		4,0,3, 4,3,7   // left
	};

	for (int i = 0; i < 36; i++)
	{
		m_Indices[i] = indices[i];
	}
}

WORLD_TRANSFORM& ModelCube::GetWorldTransform()
{
	return m_WorldTransform;
}
