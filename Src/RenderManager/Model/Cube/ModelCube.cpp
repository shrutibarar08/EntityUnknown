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
	m_PixelMetaData.DebugLine = 1;
	m_PixelMetadataCB->Update(deviceContext, &m_PixelMetaData);
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
	using namespace DirectX;

	const XMFLOAT3 positions[6][4] =
	{
		// Front (-Z)
		{ {-1,  1, -1}, { 1,  1, -1}, {-1, -1, -1}, { 1, -1, -1} },
		// Back (+Z)
		{ { 1,  1,  1}, {-1,  1,  1}, { 1, -1,  1}, {-1, -1,  1} },
		// Left (-X)
		{ {-1,  1,  1}, {-1,  1, -1}, {-1, -1,  1}, {-1, -1, -1} },
		// Right (+X)
		{ { 1,  1, -1}, { 1,  1,  1}, { 1, -1, -1}, { 1, -1,  1} },
		// Top (+Y)
		{ {-1,  1,  1}, { 1,  1,  1}, {-1,  1, -1}, { 1,  1, -1} },
		// Bottom (-Y)
		{ {-1, -1, -1}, { 1, -1, -1}, {-1, -1,  1}, { 1, -1,  1} },
	};

	const XMFLOAT2 uvs[4] = {
		{0, 0}, {1, 0}, {0, 1}, {1, 1}
	};

	const XMFLOAT3 normals[6] =
	{
		{  0,  0, -1 }, // Front
		{  0,  0,  1 }, // Back
		{ -1,  0,  0 }, // Left
		{  1,  0,  0 }, // Right
		{  0,  1,  0 }, // Top
		{  0, -1,  0 }, // Bottom
	};

	int v = 0;
	for (int face = 0; face < 6; ++face)
	{
		for (int i = 0; i < 4; ++i)
		{
			m_Vertices[v++] = {
				positions[face][i],
				uvs[i],
				normals[face] // Use flat face normal here (optionally smooth later)
			};
		}
	}

	for (int i = 0; i < 24; ++i)
	{
		XMFLOAT3 pos = m_Vertices[i].Position;
		XMVECTOR normalSum = XMVectorZero();
		int count = 0;

		for (int j = 0; j < 24; ++j)
		{
			if (m_Vertices[j].Position.x == pos.x &&
				m_Vertices[j].Position.y == pos.y &&
				m_Vertices[j].Position.z == pos.z)
			{
				normalSum = XMVectorAdd(normalSum, XMLoadFloat3(&m_Vertices[j].Normal));
				++count;
			}
		}

		XMVECTOR average = XMVector3Normalize(normalSum);
		XMStoreFloat3(&m_Vertices[i].Normal, average);
	}
}

void ModelCube::BuildIndex()
{
	// Indexing two triangles per face
	uint32_t idx = 0;
	for (uint32_t i = 0; i < 6; ++i)
	{
		uint32_t base = i * 4;
		m_Indices[idx++] = base + 0;
		m_Indices[idx++] = base + 1;
		m_Indices[idx++] = base + 2;

		m_Indices[idx++] = base + 2;
		m_Indices[idx++] = base + 1;
		m_Indices[idx++] = base + 3;
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
