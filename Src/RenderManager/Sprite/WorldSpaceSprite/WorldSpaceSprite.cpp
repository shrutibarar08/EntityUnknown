#include "WorldSpaceSprite.h"

WorldSpaceSprite::WorldSpaceSprite()
{
	EnableLight(true);
}

bool WorldSpaceSprite::Build(ID3D11Device* device)
{
	ISprite::Build(device);

	if (m_LocalInitialized) return true;
	m_LocalInitialized = true;

	//~ Shared Vertex and Index Buffer Data
	BuildVertexBuffer();
	BuildIndexBuffer();
	m_SharedVBnIB = std::make_shared<StaticVBData>(m_Vertices, m_Indices);
	m_StaticSpriteVBnIB = std::make_unique<StaticVBInstance<StaticVBData>>(m_SharedVBnIB);
	m_StaticSpriteVBnIB->Init(device);

	return true;
}

bool WorldSpaceSprite::Render(ID3D11DeviceContext* deviceContext)
{
	if (!IRender::Render(deviceContext)) return false;
	if (!m_LocalInitialized) return  false;

	m_StaticSpriteVBnIB->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#ifdef _DEBUG
	//~ Green Line for debugging;
	if (CubeCollider* collider = GetCubeCollider())
	{
		UpdatePixelMetaDataConstantBuffer(deviceContext, true);
		BindPixelMetaDataConstantBuffer(deviceContext);

		m_WorldMatrixGPU.WorldMatrix = DirectX::XMMatrixTranspose(collider->GetTransformationMatrix());
		UpdateVertexMetaDataConstantBuffer(deviceContext);
		BindVertexMetaDataConstantBuffer(deviceContext);
		m_StaticSpriteVBnIB->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}
#endif

	return true;
}

void WorldSpaceSprite::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
{
	// Get transform components
	DirectX::XMFLOAT3 scale = GetScale();
	DirectX::XMFLOAT3 translation = m_RigidBody.GetTranslation();

	// Build transformation matrix
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationZ(m_RigidBody.GetYaw());
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);

	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(S * R * T);

	// Store in world constant buffer
	m_WorldMatrixGPU.WorldMatrix = worldMatrix;
	m_WorldMatrixGPU.ViewMatrix = cameraInfo.ViewMatrix;
	m_WorldMatrixGPU.ProjectionMatrix = cameraInfo.ProjectionMatrix;
	m_WorldMatrixGPU.NormalMatrix = GetNormalTransform();
	m_WorldMatrixGPU.CameraPosition = cameraInfo.CameraPosition;
	m_WorldMatrixGPU.Padding = 0.0f;
}

bool WorldSpaceSprite::IsInitialized() const
{
	return m_LocalInitialized;
}

void WorldSpaceSprite::BuildVertexBuffer()
{
	m_Vertices[0] = { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} }; // Bottom-left
	m_Vertices[1] = { {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f} }; // Top-left
	m_Vertices[2] = { { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f} }; // Top-right

	m_Vertices[3] = { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} }; // Bottom-left (again)
	m_Vertices[4] = { { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f} }; // Top-right
	m_Vertices[5] = { { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f} }; // Bottom-right
}

void WorldSpaceSprite::BuildIndexBuffer()
{
	m_Indices[0] = 0;
	m_Indices[1] = 1;
	m_Indices[2] = 2;

	m_Indices[3] = 3;
	m_Indices[4] = 4;
	m_Indices[5] = 5;
}

void WorldSpaceSprite::BuildShaders(ID3D11Device* device)
{
	//~ Build Shaders
	m_ShaderResources.AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = m_WorldSpaceSpriteVS;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources.SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = m_WorldSpaceSpritePS;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources.SetPixelShaderPath(vertexDesc);

	m_ShaderResources.Build(device);
}
