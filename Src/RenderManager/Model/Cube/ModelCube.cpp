#include "ModelCube.h"


bool ModelCube::IsInitialized() const
{
	return m_Initialized;
}

void ModelCube::SetTextureMultiplier(int value)
{
	m_TextureMultiplier = value;
}

void ModelCube::SetTexturePath(const std::string& path)
{
	m_TexturePath = path;
}

bool ModelCube::BuildChild(ID3D11Device* device)
{
	if (m_Initialized) return true;

	BuildCubeBuffer(device);
	BuildShaders(device);

	m_Initialized = true;
	return true;
}

bool ModelCube::RenderChild(ID3D11DeviceContext* deviceContext)
{
	if (!m_Initialized) return false;

	if (m_ShaderResources->IsTextureInitialized())
	{
		m_ShaderResources->Render(deviceContext);
		m_CubeBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

#ifdef _DEBUG
	PIXEL_BUFFER_METADATA_GPU meta{};
	meta.DirectionalLightCount = 10;
	meta.DebugLine = 1;
	m_PixelMetadataCB->Update(deviceContext, &meta);
	deviceContext->PSSetConstantBuffers(0u, 1u, m_PixelMetadataCB->GetAddressOf());

	//~ Updates World Matrix Constant Buffer
	if (CubeCollider* collider = GetCubeCollider())
	{
		m_WorldMatrix.WorldMatrix = DirectX::XMMatrixTranspose(collider->GetTransformationMatrix());
		m_WorldMatrixConstantBuffer->Update(deviceContext, &m_WorldMatrix);
		deviceContext->VSSetConstantBuffers(0u, 1u, m_WorldMatrixConstantBuffer->GetAddressOf());
	}
	m_CubeBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
#endif
	return true;
}

void ModelCube::BuildVertex()
{
	std::vector<CUBE_VERTEX_DESC> cubeVertices =
	{
		{{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}},
		{{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}},
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}, { 0.0f,  0.0f, -1.0f}},
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}, { 0.0f,  0.0f, -1.0f}},
		{{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}, { 0.0f,  0.0f, -1.0f}},

		{{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}},
		{{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}},
		{{ 1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}},
		{{ 1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}},
		{{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}},
		{{ 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}},

		{{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}},
		{{-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}},
		{{ 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}},
		{{ 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}},
		{{-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}},
		{{-1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}},

		{{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}},
		{{-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}},
		{{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}},
		{{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}},
		{{-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}},
		{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}},

		{{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}},
		{{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}},
		{{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}, { 0.0f,  1.0f,  0.0f}},
		{{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}, { 0.0f,  1.0f,  0.0f}},
		{{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}},
		{{ 1.0f,  1.0f, -1.0f}, {1.0f, 1.0f}, { 0.0f,  1.0f,  0.0f}},

		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}},
		{{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}},
		{{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}, { 0.0f, -1.0f,  0.0f}},
		{{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}, { 0.0f, -1.0f,  0.0f}},
		{{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}},
		{{ 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}, { 0.0f, -1.0f,  0.0f}},
	};

	for (auto& v : cubeVertices)
	{
		v.TextureCoords.x *= m_TextureMultiplier;
		v.TextureCoords.y *= m_TextureMultiplier;
	}

	for (int i = 0; i < 36; ++i)
	{
		m_Vertices[i] = cubeVertices[i];
	}
}

void ModelCube::BuildIndex()
{
	std::vector<uint32_t> indices =
	{
		// Front face (-Z)
		0, 1, 2,  3, 4, 5,

		// Right face (+X)
		6, 7, 8,  9,10,11,

		// Back face (+Z)
		12,13,14, 15,16,17,

		// Left face (-X)
		18,19,20, 21,22,23,

		// Top face (+Y)
		24,25,26, 27,28,29,

		// Bottom face (-Y)
		30,31,32, 33,34,35
	};

	for (int i = 0; i < 36; i++)
	{
		m_Indices[i] = indices[i];
	}
}

bool ModelCube::BuildCubeBuffer(ID3D11Device* device)
{
	//~ Build Vertices
	BuildVertex();
	BuildIndex();

	m_SharedCubeBuffer = std::make_shared<CubeBuffer>(m_Vertices, m_Indices);
	m_CubeBuffer = std::make_unique<StaticVBInstance<CubeBuffer>>(m_SharedCubeBuffer);
	m_CubeBuffer->Init(device);
	return true;
}

bool ModelCube::BuildShaders(ID3D11Device* device)
{
	//~ Build Shaders
	m_ShaderResources = std::make_unique<ShaderResource>();
	m_ShaderResources->AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources->AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	m_ShaderResources->AddElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	if (!m_TexturePath.empty()) m_ShaderResources->SetTexture(m_TexturePath);

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

	if (!m_ShaderResources->Build(device)) return false;
	return true;
}
