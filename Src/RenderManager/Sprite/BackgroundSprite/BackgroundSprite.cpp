#include "BackgroundSprite.h"

#include "Imgui/imgui.h"
#include "Utils/Logger/Logger.h"

#include "Editor/Core/UiPolicy/WidgetPolicy/ContentBrowser/ImGuiContentBrowserPolicy.h"

BackgroundSprite::BackgroundSprite()
{
	GetCubeCollider()->SetColliderState(ColliderState::Trigger);
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

bool BackgroundSprite::Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	if (!ISprite::Build(device, deviceContext)) return false;

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

void BackgroundSprite::RenderControlUI(LevelEditorContext* context)
{
    using CB = ImGuiContentBrowserPolicy;

    auto SafeCopy = [](char* dst, size_t dstSize, const std::string& src)
        {
            if (!dst || dstSize == 0) return;
            std::memset(dst, 0, dstSize);
            const size_t n = std::min(src.size(), dstSize - 1);
            if (n) std::memcpy(dst, src.data(), n);
            dst[dstSize - 1] = '\0';
        };
    auto Log = [](const std::string& s) { LOG_INFO(s); };

    // === Name Control ===
    static char nameBuffer[128]{};
    static uintptr_t lastObjectID = 0;
    uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
    if (lastObjectID != currentID)
    {
        lastObjectID = currentID;
        SafeCopy(nameBuffer, sizeof(nameBuffer), GetName());
        LOG_INFO("[Select] BackgroundSprite selected; reset UI buffers");
    }
    if (ImGui::InputText("Object Name", nameBuffer, sizeof(nameBuffer)))
        LOG_INFO(std::string("[Edit] Name = '") + nameBuffer + "'");
    ImGui::SameLine();
    if (ImGui::Button("Rename"))
    {
        LOG_INFO(std::string("[Rename] -> '") + nameBuffer + "'");
        SetName(nameBuffer);
    }

    ImGui::Separator();
    ImGui::Text("Shader Textures (TGA Only) — drag paths here, then Apply");

    // === Texture Path Buffers ===
    static char textureBuffers[12][256]{};
    static bool initialized = false;
    if (!initialized || lastObjectID != currentID)
    {
        auto& shader = m_ShaderResources;
        SafeCopy(textureBuffers[0], sizeof(textureBuffers[0]), shader.GetTexture());
        SafeCopy(textureBuffers[1], sizeof(textureBuffers[1]), shader.GetSecondaryTexture());
        SafeCopy(textureBuffers[2], sizeof(textureBuffers[2]), shader.GetLightMap());
        SafeCopy(textureBuffers[3], sizeof(textureBuffers[3]), shader.GetAlphaMap());
        SafeCopy(textureBuffers[4], sizeof(textureBuffers[4]), shader.GetNormalMap());
        SafeCopy(textureBuffers[5], sizeof(textureBuffers[5]), shader.GetHeightMap());
        SafeCopy(textureBuffers[6], sizeof(textureBuffers[6]), shader.GetRoughnessMap());
        SafeCopy(textureBuffers[7], sizeof(textureBuffers[7]), shader.GetMetalnessMap());
        SafeCopy(textureBuffers[8], sizeof(textureBuffers[8]), shader.GetAOMap());
        SafeCopy(textureBuffers[9], sizeof(textureBuffers[9]), shader.GetSpecularMap());
        SafeCopy(textureBuffers[10], sizeof(textureBuffers[10]), shader.GetEmissiveMap());
        SafeCopy(textureBuffers[11], sizeof(textureBuffers[11]), shader.GetDisplacementMap());
        initialized = true;
        LOG_INFO("[Init] BackgroundSprite texture fields mirrored from shader");
    }

    const char* labels[12] = 
    {
        "Texture", "Secondary Texture", "Light Map", "Alpha Map", "Normal Map",
        "Height Map", "Roughness Map", "Metalness Map", "AO Map", "Specular Map",
        "Emissive Map", "Displacement Map"
    };

    // helper: VERTICAL layout => label on its own line, then DnD/Input field
    auto PathRowWithDnD_Vert = [&](int idx, const char* label)
        {
            // Label line
            ImGui::TextUnformatted(label);

            // Field line (hidden label so it doesn't render twice)
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            const std::string hidden = std::string("##") + label; // unique ID
            if (ImGui::InputText(hidden.c_str(), textureBuffers[idx], sizeof(textureBuffers[idx])))
            {
                LOG_INFO(std::string("[Edit] ") + label + " = '" + std::string(textureBuffers[idx]) + "'");
            }

            // Accept DnD on the input item
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(CB::kPayloadType))
                {
                    CB::PayloadHeader hdr{};
                    std::string pathUtf8;
                    if (CB::ParsePayload(payload, hdr, pathUtf8))
                    {
                        if ((CB::Kind)hdr.kind == CB::Kind::File)
                        {
                            SafeCopy(textureBuffers[idx], sizeof(textureBuffers[idx]), pathUtf8);
                            LOG_INFO(std::string("[DnD] ") + label + " <- '" + pathUtf8 + "'");
                        }
                        else
                        {
                            LOG_INFO(std::string("[DnD] Ignored non-file for ") + label);
                        }
                    }
                    else
                    {
                        LOG_INFO(std::string("[DnD] ParsePayload failed for ") + label);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // a little vertical spacing between entries
            ImGui::Spacing();
        };

    for (int i = 0; i < 12; ++i)
        PathRowWithDnD_Vert(i, labels[i]);

    // === Apply Button ===
    if (ImGui::Button("Apply Textures"))
    {
        auto& shader = m_ShaderResources;

        // Log all paths before applying
        for (int i = 0; i < 12; ++i)
            LOG_INFO(std::string("[Apply] ") + labels[i] + " = '" + std::string(textureBuffers[i]) + "'");

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

        LOG_INFO("[Apply] BackgroundSprite textures committed to shader resources");
    }

    // === Alpha Value ===
    {
        float alpha = m_ShaderResources.GetAlphaValue();
        if (ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f))
        {
            m_ShaderResources.SetAlphaValue(alpha);
            LOG_INFO("[Edit] Alpha changed");
        }

        bool transparent = IsTransparent();
        if (ImGui::Checkbox("Transparent", &transparent))
        {
            SetTransparent(transparent);
            LOG_INFO(std::string("[Edit] Transparent = ") + (transparent ? "true" : "false"));
        }
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

    if (ImGui::DragFloat("Left", &left, 0.01f, 0.0f, 1.0f)) { SetLeftPercent(left);   LOG_INFO("[Edit] Left%"); }
    if (ImGui::DragFloat("Right", &right, 0.01f, 0.0f, 1.0f)) { SetRightPercent(right); LOG_INFO("[Edit] Right%"); }
    if (ImGui::DragFloat("Top", &top, 0.01f, 0.0f, 1.0f)) { SetTopPercent(top);     LOG_INFO("[Edit] Top%"); }
    if (ImGui::DragFloat("Bottom", &bottom, 0.01f, 0.0f, 1.0f)) { SetDownPercent(bottom); LOG_INFO("[Edit] Bottom%"); }

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

void BackgroundSprite::LoadRenderSaveData(const nlohmann::json& json)
{
	ISprite::LoadRenderSaveData(json);
	if (!json.is_object()) return;

	auto readF = [&](const char* key, float& dst)
	{
		auto it = json.find(key);
		if (it == json.end()) return;

		if (it->is_number_float())        dst = it->get<float>();
		else if (it->is_number_integer()) dst = static_cast<float>(it->get<long long>());
		else if (it->is_string()) { try { dst = std::stof(it->get<std::string>()); } catch (...) {} }
	};

	readF("LeftPercent",  m_LeftPercent);
	readF("RightPercent", m_RightPercent);
	readF("TopPercent",   m_TopPercent);
	readF("DownPercent",  m_DownPercent);
}

nlohmann::json BackgroundSprite::GetRenderSaveData() const
{
	nlohmann::json data = ISprite::GetRenderSaveData();

	data["LeftPercent"	] = m_LeftPercent;
	data["RightPercent"	] = m_RightPercent;
	data["TopPercent"	] = m_TopPercent;
	data["DownPercent"	] = m_DownPercent;

	return data;
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

void BackgroundSprite::BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//~ Build Shaders
	m_ShaderResources.AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

	EU_BLOB_INIT_DESC vertexDesc{};
	vertexDesc.FilePath = m_BackgroundVertexShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources.SetVertexShaderPath(vertexDesc);

	EU_BLOB_INIT_DESC PixelDesc{};
	vertexDesc.FilePath = m_BackgroundPixelShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources.SetPixelShaderPath(vertexDesc);

	m_ShaderResources.Build(device, deviceContext);
}

void BackgroundSprite::RenderGeometry(ID3D11DeviceContext* deviceContext)
{
	if (!m_LocalInitialized) return;
	UpdateVertexBuffer(deviceContext);
	m_DynamicSpriteBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
