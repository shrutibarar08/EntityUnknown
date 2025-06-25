#include "ScreenSprite.h"

#include "Imgui/imgui.h"
#include "Utils/Logger/Logger.h"


ScreenSprite::ScreenSprite()
{
	EnableLight(false);
}

void ScreenSprite::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
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

bool ScreenSprite::IsInitialized() const
{
	return m_LocalInitialized;
}

bool ScreenSprite::Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	ISprite::Build(device, deviceContext);

	if (m_LocalInitialized) return true;
	m_LocalInitialized = true;

	//~ Shared Data
	m_SharedBitMapBuffer = std::make_unique<DynamicVBnIB>(6, 6);
	m_DynamicSpriteBuffer = std::make_unique<DynamicInstance<DynamicVBnIB>>(m_SharedBitMapBuffer);
	m_DynamicSpriteBuffer->Init(device);

	return true;
}

bool ScreenSprite::Render(ID3D11DeviceContext* deviceContext)
{
	if (!m_LocalInitialized) return false;
	ISprite::Render(deviceContext);

	UpdateVertexBuffer(deviceContext);
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

void ScreenSprite::BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//~ Build Shaders
	m_ShaderResources.AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = m_ScreenSpriteVertexShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources.SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = m_ScreenSpritePixelShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources.SetPixelShaderPath(vertexDesc);

	m_ShaderResources.Build(device, deviceContext);
}

void ScreenSprite::RenderGeometry(ID3D11DeviceContext* deviceContext)
{
	if (!m_LocalInitialized) return;
	UpdateVertexBuffer(deviceContext);
	m_DynamicSpriteBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ScreenSprite::RenderControlUI()
{
	// === Name Control ===
	static char nameBuffer[128]{};
	static uintptr_t lastObjectID = 0;
	uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
	if (lastObjectID != currentID)
	{
		lastObjectID = currentID;
		strncpy_s(nameBuffer, GetName().c_str(), sizeof(nameBuffer));
	}
	ImGui::InputText("Object Name", nameBuffer, sizeof(nameBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Rename"))
		SetName(nameBuffer);

	ImGui::Separator();
	ImGui::Text("Shader Textures (TGA Only)");

	// === Texture Path Buffers ===
	static char textureBuffers[12][256]{};
	static bool initialized = false;
	if (!initialized || lastObjectID != currentID)
	{
		auto& shader = m_ShaderResources;
		strncpy_s(textureBuffers[0], shader.GetTexture().c_str(), sizeof(textureBuffers[0]));
		strncpy_s(textureBuffers[1], shader.GetSecondaryTexture().c_str(), sizeof(textureBuffers[1]));
		strncpy_s(textureBuffers[2], shader.GetLightMap().c_str(), sizeof(textureBuffers[2]));
		strncpy_s(textureBuffers[3], shader.GetAlphaMap().c_str(), sizeof(textureBuffers[3]));
		strncpy_s(textureBuffers[4], shader.GetNormalMap().c_str(), sizeof(textureBuffers[4]));
		strncpy_s(textureBuffers[5], shader.GetHeightMap().c_str(), sizeof(textureBuffers[5]));
		strncpy_s(textureBuffers[6], shader.GetRoughnessMap().c_str(), sizeof(textureBuffers[6]));
		strncpy_s(textureBuffers[7], shader.GetMetalnessMap().c_str(), sizeof(textureBuffers[7]));
		strncpy_s(textureBuffers[8], shader.GetAOMap().c_str(), sizeof(textureBuffers[8]));
		strncpy_s(textureBuffers[9], shader.GetSpecularMap().c_str(), sizeof(textureBuffers[9]));
		strncpy_s(textureBuffers[10], shader.GetEmissiveMap().c_str(), sizeof(textureBuffers[10]));
		strncpy_s(textureBuffers[11], shader.GetDisplacementMap().c_str(), sizeof(textureBuffers[11]));
		initialized = true;
	}

	const char* labels[] = {
		"Texture", "Secondary Texture", "Light Map", "Alpha Map", "Normal Map",
		"Height Map", "Roughness Map", "Metalness Map", "AO Map", "Specular Map",
		"Emissive Map", "Displacement Map"
	};

	for (int i = 0; i < 12; ++i)
	{
		ImGui::InputText(labels[i], textureBuffers[i], sizeof(textureBuffers[i]));
		ImGui::SameLine();
		std::string buttonLabel = "Browse##" + std::to_string(i);
		if (ImGui::Button(buttonLabel.c_str()))
		{
			std::string path = OpenFileDialog("TGA Files\0*.tga\0All Files\0*.*\0");
			if (!path.empty())
				strncpy_s(textureBuffers[i], path.c_str(), sizeof(textureBuffers[i]));
		}
	}

	// === Apply Button ===
	if (ImGui::Button("Apply Textures"))
	{
		auto& shader = m_ShaderResources;
		shader.SetTexture(textureBuffers[0]);
		shader.SetSecondaryTexture(textureBuffers[1]);
		shader.SetLightMap(textureBuffers[2]);
		shader.SetAlphaMap(textureBuffers[3]);
		shader.SetNormalMap(textureBuffers[4]);
		shader.SetHeightMap(textureBuffers[5]);
		shader.SetRoughnessMap(textureBuffers[6]);
		shader.SetMetalnessMap(textureBuffers[7]);
		shader.SetAOMap(textureBuffers[8]);
		shader.SetSpecularMap(textureBuffers[9]);
		shader.SetEmissiveMap(textureBuffers[10]);
		shader.SetDisplacementMap(textureBuffers[11]);
	}

	ImGui::Separator();
	ImGui::Text("Screen Bounds (%)");

	// === Percent Controls - Live Update ===
	static float left, right, top, bottom;
	if (lastObjectID != currentID)
	{
		left = GetLeftPercent();
		right = GetRightPercent();
		top = GetTopPercent();
		bottom = GetDownPercent();
	}

	if (ImGui::DragFloat("Left", &left, 0.01f, 0.0f, 1.0f)) SetLeftPercent(left);
	if (ImGui::DragFloat("Right", &right, 0.01f, 0.0f, 1.0f)) SetRightPercent(right);
	if (ImGui::DragFloat("Top", &top, 0.01f, 0.0f, 1.0f)) SetTopPercent(top);
	if (ImGui::DragFloat("Bottom", &bottom, 0.01f, 0.0f, 1.0f)) SetDownPercent(bottom);

	ImGui::Separator();
	ImGui::Text("Transform");

	DirectX::XMFLOAT3 pos = m_RigidBody.GetTranslation();
	if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
		m_RigidBody.SetTranslation(pos.x, pos.y, pos.z);

	Quaternion q = m_RigidBody.GetOrientation();
	float orientation[4] = { q.GetI(), q.GetJ(), q.GetK(), q.GetR() };
	if (ImGui::DragFloat4("Orientation (x, y, z, w)", orientation, 0.01f))
		m_RigidBody.SetOrientation({ orientation[3], orientation[0], orientation[1], orientation[2] });
}
