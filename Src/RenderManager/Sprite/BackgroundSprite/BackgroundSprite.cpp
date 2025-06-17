#include "BackgroundSprite.h"

#include "Utils/Logger/Logger.h"

BackgroundSprite::BackgroundSprite()
{
	EnableLight(true);
	m_LightManager.SetActivePointLight(false);
	m_LightManager.SetActiveSpotLight(false);
	m_LightManager.SetActiveDirectionalLight(true);
}

void BackgroundSprite::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
{
	// Optional scale/rotation in clip-space
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationZ(m_RigidBody.GetYaw());

	// Translation not needed if vertices are in NDC
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(R);

	m_WorldMatrixGPU.WorldMatrix = worldMatrix;
	m_WorldMatrixGPU.ViewMatrix = DirectX::XMMatrixIdentity();
	m_WorldMatrixGPU.ProjectionMatrix = DirectX::XMMatrixIdentity();
	m_WorldMatrixGPU.NormalMatrix = GetNormalTransform();
	m_WorldMatrixGPU.CameraPosition = cameraInfo.CameraPosition;
	m_WorldMatrixGPU.Padding = 0.f;
}

bool BackgroundSprite::IsInitialized() const
{
	return m_LocalInitialized;
}

bool BackgroundSprite::Build(ID3D11Device* device)
{
	if (!ISprite::Build(device)) return false;

	if (m_LocalInitialized) return true;
	m_LocalInitialized = true;

	//~ Shared Data
	m_SharedBitMapBuffer = std::make_unique<DynamicVBnIB>(6, 6);
	m_DynamicSpriteBuffer = std::make_unique<DynamicInstance<DynamicVBnIB>>(m_SharedBitMapBuffer);
	m_DynamicSpriteBuffer->Init(device);

	return true;
}

bool BackgroundSprite::Render(ID3D11DeviceContext* deviceContext)
{
	if (!ISprite::Render(deviceContext)) return false;
	if (!m_LocalInitialized) return false;

	UpdateVertexBuffer(deviceContext);
	m_DynamicSpriteBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return true;
}

void BackgroundSprite::UpdateVertexBuffer(ID3D11DeviceContext* deviceContext)
{
	float posX = m_RigidBody.GetTranslation().x;
	float posY = m_RigidBody.GetTranslation().y;

	if (!m_bDirty && posX == m_LastX && posY == m_LastY)
	{
		return;
	}

	LOG_INFO("Updated Vertex Buffer with: " + std::to_string(posX) + ", " + std::to_string(posY) + " Dirty Flag: " + std::to_string(m_bDirty));
	LOG_INFO("Actual Updated Vertex Buffer with: " + std::to_string(m_LastX) + ", " + std::to_string(m_LastY) + " Dirty Flag: " + std::to_string(m_bDirty));

	m_LastX = posX;
	m_LastY = posY;
	m_bDirty = false;

	TEXTURE_RESOURCE resource = m_ShaderResources.GetTextureResource();
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

void BackgroundSprite::BuildShaders(ID3D11Device* device)
{
	//~ Build Shaders
	m_ShaderResources.AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = m_BackgroundVertexShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources.SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = m_BackgroundPixelShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources.SetPixelShaderPath(vertexDesc);

	m_ShaderResources.Build(device);
}
