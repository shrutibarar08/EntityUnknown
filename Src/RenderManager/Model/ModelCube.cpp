#include "ModelCube.h"

inline void PrintMatrix(const char* label, const DirectX::XMMATRIX& mat)
{
	using namespace DirectX;
	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, mat);

	printf("[%s Matrix]\n", label);
	printf("| % .3f % .3f % .3f % .3f |\n", m._11, m._12, m._13, m._14);
	printf("| % .3f % .3f % .3f % .3f |\n", m._21, m._22, m._23, m._24);
	printf("| % .3f % .3f % .3f % .3f |\n", m._31, m._32, m._33, m._34);
	printf("| % .3f % .3f % .3f % .3f |\n", m._41, m._42, m._43, m._44);
	printf("\n");
}

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
	m_ShaderResources->AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	m_ShaderResources->AddTexture("Texture/sample.tga");

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

	std::string message = "Initialized Cube Model: Index Count: " + std::to_string(m_CubeBuffer->GetIndexCount());
	LOG_INFO(message);

	return true;
}

bool ModelCube::Render(ID3D11DeviceContext* deviceContext)
{
	if (!m_Initialized) return false;

	m_ShaderResources->Render(deviceContext);
	m_VertexConstantBuffer->Update(deviceContext, &m_WorldTransform);
	deviceContext->VSSetConstantBuffers(
		0u,
		1u,
		m_VertexConstantBuffer->GetAddressOf()
	);
	m_CubeBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CubeBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	return true;
}

void ModelCube::UpdateTransformation(const CAMERA_MATRIX_DESC* cameraInfo)
{
	m_WorldTransform.ProjectionMatrix = cameraInfo->ProjectionMatrix;
	m_WorldTransform.ViewMatrix = cameraInfo->ViewMatrix;
	m_WorldTransform.TransformationMatrix = GetTransform();
}

bool ModelCube::IsInitialized() const
{
	return m_Initialized;
}

void ModelCube::BuildVertex()
{
	std::vector<CUBE_VERTEX_DESC> vertices =
	{
		// Front face (-Z)
		{{-1, -1, -1}, {0, 0, -1}, {1, 0, 0, 1}, {0.0f, 1.0f}}, // 0
		{{ 1, -1, -1}, {0, 0, -1}, {1, 0, 0, 1}, {1.0f, 1.0f}}, // 1
		{{ 1,  1, -1}, {0, 0, -1}, {1, 0, 0, 1}, {1.0f, 0.0f}}, // 2
		{{-1,  1, -1}, {0, 0, -1}, {1, 0, 0, 1}, {0.0f, 0.0f}}, // 3

		// Back face (+Z)
		{ { 1, -1,  1}, {0, 0, 1}, {0, 1, 0, 1}, {0.0f, 1.0f} },
		{ {-1, -1,  1}, {0, 0, 1}, {0, 1, 0, 1}, {1.0f, 1.0f} },
		{ {-1,  1,  1}, {0, 0, 1}, {0, 1, 0, 1}, {1.0f, 0.0f} },
		{ { 1,  1,  1}, {0, 0, 1}, {0, 1, 0, 1}, {0.0f, 0.0f} },

		// Left face (-X)
		{ {-1, -1,  1}, {-1, 0, 0}, {0, 0, 1, 1}, {0.0f, 1.0f} },
		{ {-1, -1, -1}, {-1, 0, 0}, {0, 0, 1, 1}, {1.0f, 1.0f} },
		{ {-1,  1, -1}, {-1, 0, 0}, {0, 0, 1, 1}, {1.0f, 0.0f} },
		{ {-1,  1,  1}, {-1, 0, 0}, {0, 0, 1, 1}, {0.0f, 0.0f} },

		// Right face (+X)
		{ {1, -1, -1}, {1, 0, 0}, {1, 1, 0, 1}, {0.0f, 1.0f} },
		{ {1, -1,  1}, {1, 0, 0}, {1, 1, 0, 1}, {1.0f, 1.0f} },
		{ {1,  1,  1}, {1, 0, 0}, {1, 1, 0, 1}, {1.0f, 0.0f} },
		{ {1,  1, -1}, {1, 0, 0}, {1, 1, 0, 1}, {0.0f, 0.0f} },

		// Top face (+Y)
		{ {-1, 1, -1}, {0, 1, 0}, {1, 0, 1, 1}, {0.0f, 1.0f} },
		{ { 1, 1, -1}, {0, 1, 0}, {1, 0, 1, 1}, {1.0f, 1.0f} },
		{ { 1, 1,  1}, {0, 1, 0}, {1, 0, 1, 1}, {1.0f, 0.0f} },
		{ {-1, 1,  1}, {0, 1, 0}, {1, 0, 1, 1}, {0.0f, 0.0f} },

		// Bottom face (-Y)
		{ {-1, -1,  1}, {0, -1, 0}, {0, 1, 1, 1}, {0.0f, 1.0f} },
		{ { 1, -1,  1}, {0, -1, 0}, {0, 1, 1, 1}, {1.0f, 1.0f} },
		{ { 1, -1, -1}, {0, -1, 0}, {0, 1, 1, 1}, {1.0f, 0.0f} },
		{ {-1, -1, -1}, {0, -1, 0}, {0, 1, 1, 1}, {0.0f, 0.0f} },
	};

	for (int i = 0; i < 24; ++i)
	{
		m_Vertices[i] = vertices[i];
	}
}

void ModelCube::BuildIndex()
{
	std::vector<uint32_t> indices =
	{
		// Front face
		0, 1, 2,  0, 2, 3,
		// Back face
		4, 5, 6,  4, 6, 7,
		// Left face
		8, 9,10,  8,10,11,
		// Right face
		12,13,14, 12,14,15,
		// Top face
		16,17,18, 16,18,19,
		// Bottom face
		20,21,22, 20,22,23
	};

	for (int i = 0; i < 36; i++)
	{
		m_Indices[i] = indices[i];
	}
}
