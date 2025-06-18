#include "ModelCube.h"

bool ModelCube::IsInitialized() const
{
	return m_Initialized;
}

void ModelCube::SetTextureMultiplier(int valueX, int valueY)
{
	m_TextureMultiplierX = valueX;
	m_TextureMultiplierY = valueY;
}

bool ModelCube::BuildChild(ID3D11Device* device)
{
	if (m_Initialized) return true;
	BuildCubeBuffer(device);
	m_Initialized = true;

	return true;
}

bool ModelCube::RenderChild(ID3D11DeviceContext* deviceContext)
{
	if (!m_Initialized) return false;

	m_CubeBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#ifdef _DEBUG

	// Render Debug Lines
	if (CubeCollider* collider = GetCubeCollider())
	{
		UpdatePixelMetaDataConstantBuffer(deviceContext, true);
		BindPixelMetaDataConstantBuffer(deviceContext);

		m_WorldMatrixGPU.WorldMatrix = DirectX::XMMatrixTranspose(collider->GetTransformationMatrix());
		UpdateVertexMetaDataConstantBuffer(deviceContext);
		BindVertexMetaDataConstantBuffer(deviceContext);

		m_CubeBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
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

	const XMFLOAT2 baseUVs[4] = {
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
		XMVECTOR tangent = XMVectorZero();
		XMVECTOR binormal = XMVectorZero();

		// Extract positions and scaled UVs
		XMVECTOR p0 = XMLoadFloat3(&positions[face][0]);
		XMVECTOR p1 = XMLoadFloat3(&positions[face][1]);
		XMVECTOR p2 = XMLoadFloat3(&positions[face][2]);

		XMFLOAT2 uv0 = { baseUVs[0].x * m_TextureMultiplierX, baseUVs[0].y * m_TextureMultiplierY };
		XMFLOAT2 uv1 = { baseUVs[1].x * m_TextureMultiplierX, baseUVs[1].y * m_TextureMultiplierY };
		XMFLOAT2 uv2 = { baseUVs[2].x * m_TextureMultiplierX, baseUVs[2].y * m_TextureMultiplierY };

		// Tangent space calculation (MikkTSpace style)
		XMVECTOR edge1 = XMVectorSubtract(p1, p0);
		XMVECTOR edge2 = XMVectorSubtract(p2, p0);

		float du1 = uv1.x - uv0.x;
		float dv1 = uv1.y - uv0.y;
		float du2 = uv2.x - uv0.x;
		float dv2 = uv2.y - uv0.y;

		float f = 1.0f / (du1 * dv2 - du2 * dv1);

		tangent = XMVectorSet(
			f * (dv2 * XMVectorGetX(edge1) - dv1 * XMVectorGetX(edge2)),
			f * (dv2 * XMVectorGetY(edge1) - dv1 * XMVectorGetY(edge2)),
			f * (dv2 * XMVectorGetZ(edge1) - dv1 * XMVectorGetZ(edge2)),
			0.0f
		);
		binormal = XMVectorSet(
			f * (-du2 * XMVectorGetX(edge1) + du1 * XMVectorGetX(edge2)),
			f * (-du2 * XMVectorGetY(edge1) + du1 * XMVectorGetY(edge2)),
			f * (-du2 * XMVectorGetZ(edge1) + du1 * XMVectorGetZ(edge2)),
			0.0f
		);

		tangent = XMVector3Normalize(tangent);
		binormal = XMVector3Normalize(binormal);

		XMFLOAT3 tangentF, binormalF;
		XMStoreFloat3(&tangentF, tangent);
		XMStoreFloat3(&binormalF, binormal);

		// Populate all 4 vertices of the face
		for (int i = 0; i < 4; ++i)
		{
			XMFLOAT2 uv = {
				baseUVs[i].x * m_TextureMultiplierX,
				baseUVs[i].y * m_TextureMultiplierY
			};

			m_Vertices[v++] = {
				positions[face][i],
				uv,
				normals[face],
				tangentF,
				binormalF
			};
		}
	}

	// Optional: smooth normals pass (skip if you're using flat shading)
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
	uint32_t idx = 0;
	for (uint32_t i = 0; i < 6; ++i)
	{
		uint32_t base = i * 4;

		// First triangle of the face
		m_Indices[idx++] = base + 0;
		m_Indices[idx++] = base + 1;
		m_Indices[idx++] = base + 2;

		// Second triangle of the face
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

void ModelCube::BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//~ Build Shaders
	m_ShaderResources.AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	m_ShaderResources.AddElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("BINORMAL", DXGI_FORMAT_R32G32B32_FLOAT);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = L"Shader/Shape/CubeShaderVS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources.SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = L"Shader/Shape/CubeShaderPS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources.SetPixelShaderPath(vertexDesc);

	m_ShaderResources.Build(device, deviceContext);
}
