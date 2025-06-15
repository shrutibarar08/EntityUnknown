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

	//~ Build Shaders
	m_ShaderResource = std::make_unique<ShaderResource>();
	m_ShaderResource->AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResource->AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if (!m_TextureResource.IsInitialized()) m_ShaderResource->SetTexture(m_TexturePath);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = m_VertexShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResource->SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = m_PixelShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResource->SetPixelShaderPath(vertexDesc);

	if (!m_ShaderResource->Build(device))
	{
		return false;
	}

	return true;
}

bool WorldSpaceSprite::Render(ID3D11DeviceContext* deviceContext)
{
	ISprite::Render(deviceContext);
	m_ShaderResource->Render(deviceContext);
	m_StaticSpriteVBnIB->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return true;
}

void WorldSpaceSprite::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
{
	// Get transform components
	DirectX::XMFLOAT3 scale = GetScale();
	DirectX::XMFLOAT3 translation = m_RigidBody.GetTranslation();
	DirectX::XMFLOAT3 rotation = m_RigidBody.GetRotation(); // pitch (X), yaw (Y), roll (Z)

	// Build transformation matrix
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);

	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(S * R * T);

	// Store in world constant buffer
	m_WorldMatrix.WorldMatrix = worldMatrix;
	m_WorldMatrix.ViewMatrix = cameraInfo.ViewMatrix;
	m_WorldMatrix.ProjectionMatrix = cameraInfo.ProjectionMatrix;
	m_WorldMatrix.CameraPosition = cameraInfo.CameraPosition;
	m_WorldMatrix.Padding = 0.0f;
}

bool WorldSpaceSprite::IsInitialized() const
{
	return m_LocalInitialized;
}

void WorldSpaceSprite::UpdateTextureResource(const TEXTURE_RESOURCE& resource)
{
	m_TextureResource = resource;
	if (m_ShaderResource && m_TextureResource.IsInitialized())
	{
		m_ShaderResource->SetTexture(m_TextureResource);
	}
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
