#include "ScreenSprite.h"

#include "Utils/Logger/Logger.h"


ScreenSprite::ScreenSprite()
{
	m_VertexShaderPath = L"Shader/Sprite/ScreenSprite/VertexShader.hlsl";
	m_PixelShaderPath = L"Shader/Sprite/ScreenSprite/PixelShader.hlsl";
	EnableLight(false);
}

void ScreenSprite::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
{
	// Optional scale/rotation in clip-space
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationZ(m_RigidBody.GetYaw());

	// Translation not needed if vertices are in NDC
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(R);

	m_WorldMatrix.WorldMatrix = worldMatrix;
	m_WorldMatrix.ViewMatrix = DirectX::XMMatrixIdentity();
	m_WorldMatrix.ProjectionMatrix = DirectX::XMMatrixIdentity();
	m_WorldMatrix.CameraPosition = { 0.f, 0.f, 0.f }; // unused
	m_WorldMatrix.Padding = 0.f;
}

bool ScreenSprite::IsInitialized() const
{
	return m_LocalInitialized;
}

void ScreenSprite::UpdateTextureResource(const TEXTURE_RESOURCE& resource)
{
	m_TextureResource = resource;
	if (m_ShaderResources && m_TextureResource.IsInitialized())
	{
		m_ShaderResources->SetTexture(m_TextureResource);
	}
}

bool ScreenSprite::Build(ID3D11Device* device)
{
	ISprite::Build(device);

	if (m_LocalInitialized) return true;
	m_LocalInitialized = true;

	//~ Shared Data
	m_SharedBitMapBuffer = std::make_unique<DynamicVBnIB>(6, 6);
	m_DynamicSpriteBuffer = std::make_unique<DynamicInstance<DynamicVBnIB>>(m_SharedBitMapBuffer);
	m_DynamicSpriteBuffer->Init(device);

	//~ Build Shaders
	m_ShaderResources = std::make_unique<ShaderResource>();
	m_ShaderResources->AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources->AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	if (!m_TextureResource.IsInitialized()) m_ShaderResources->SetTexture(m_TexturePath);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = m_VertexShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources->SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = m_PixelShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources->SetPixelShaderPath(vertexDesc);

	if (!m_ShaderResources->Build(device))
	{
		return false;
	}

	return true;
}

bool ScreenSprite::Render(ID3D11DeviceContext* deviceContext)
{
	if (!m_LocalInitialized) return false;
	ISprite::Render(deviceContext);

	UpdateVertexBuffer(deviceContext);
	m_ShaderResources->Render(deviceContext);
	m_DynamicSpriteBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return true;
}

void ScreenSprite::UpdateVertexBuffer(ID3D11DeviceContext* deviceContext)
{
	float posX = m_RigidBody.GetTranslation().x;
	float posY = m_RigidBody.GetTranslation().y;

	if ((posX == m_LastX && posY == m_LastY) && !m_bDirty) return;

	m_LastX = posX;
	m_LastY = posY;
	m_bDirty = false;

	TEXTURE_RESOURCE resource = m_ShaderResources->GetTextureResource();
	if (!resource.IsInitialized()) return;

	// Total size in pixels for X and Y
	float halfScreenWidth = 0.5f * static_cast<float>(m_ScreenWidth);
	float halfScreenHeight = 0.5f * static_cast<float>(m_ScreenHeight);

	// Percent to pixels (relative to center)
	float leftPixels = -halfScreenWidth * m_LeftPercent;
	float rightPixels = halfScreenWidth * m_RightPercent;
	float topPixels = halfScreenHeight * m_TopPercent;
	float bottomPixels = -halfScreenHeight * m_DownPercent;

	// World space position offset
	float centerX = m_RigidBody.GetTranslation().x;
	float centerY = m_RigidBody.GetTranslation().y;

	// Final positions in pixels
	float left = centerX + leftPixels;
	float right = centerX + rightPixels;
	float top = centerY + topPixels;
	float bottom = centerY + bottomPixels;

	// Convert to NDC
	float ndcLeft = (left / halfScreenWidth);
	float ndcRight = (right / halfScreenWidth);
	float ndcTop = (top / halfScreenHeight);
	float ndcBottom = (bottom / halfScreenHeight);

	std::vector<Vertex2D> vertices(6);
	vertices[0] = { {ndcLeft,  ndcTop,    0.0f}, {0.0f, 0.0f} }; // TL
	vertices[1] = { {ndcRight, ndcBottom, 0.0f}, {1.0f, 1.0f} }; // BR
	vertices[2] = { {ndcLeft,  ndcBottom, 0.0f}, {0.0f, 1.0f} }; // BL
	vertices[3] = { {ndcLeft,  ndcTop,    0.0f}, {0.0f, 0.0f} };
	vertices[4] = { {ndcRight, ndcTop,    0.0f}, {1.0f, 0.0f} };
	vertices[5] = { {ndcRight, ndcBottom, 0.0f}, {1.0f, 1.0f} };

	m_DynamicSpriteBuffer->Update(deviceContext, vertices);
}

bool ScreenSprite::IsMultiTextureEnable() const
{
	if (m_ShaderResources)
	{
		return m_ShaderResources->IsOptionalTextureInitialized();
	}
	return false;
}
