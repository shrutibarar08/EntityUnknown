#include "Mesh.h"

#include "ExceptionManager/IException.h"
#include "Imgui/imgui.h"
#include "RenderManager/Model/ModelLoader/ObjLoader/ObjLoader.h"

#include "Editor/Core/UiPolicy/WidgetPolicy/ContentBrowser/ImGuiContentBrowserPolicy.h"


void Mesh::SetMeshPath(const std::string& path)
{
	m_MeshPath = path;
}

bool Mesh::IsInitialized() const
{
	return m_Initialized;
}

void Mesh::RenderControlUI(LevelEditorContext* context)
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

    static char nameBuffer[128]{};
    static uintptr_t lastObjectID = 0;
    uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
    if (lastObjectID != currentID)
    {
        lastObjectID = currentID;
        SafeCopy(nameBuffer, sizeof(nameBuffer), GetName());
        Log("[Select] Mesh selected; reset UI buffers");
    }
    if (ImGui::InputText("Object Name", nameBuffer, sizeof(nameBuffer)))
        Log(std::string("[Edit] Name = '") + nameBuffer + "'");
    ImGui::SameLine();
    if (ImGui::Button("Rename"))
    {
        Log(std::string("[Rename] -> '") + nameBuffer + "'");
        SetName(std::string(nameBuffer));
    }

    ImGui::Separator();
    ImGui::Text("Shader Textures & Mesh Path");

    static char meshBuffer[256]{};
    static char textureBuffers[12][256]{};
    static uintptr_t lastPathObjectID = 0;
    if (lastPathObjectID != currentID)
    {
        lastPathObjectID = currentID;
        SafeCopy(meshBuffer, sizeof(meshBuffer), m_MeshPath);
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
        Log("[Init] Mesh and texture fields mirrored from resources");
    }

    const char* labels[12] = {
        "Texture", "Secondary Texture", "Light Map", "Alpha Map", "Normal Map",
        "Height Map", "Roughness Map", "Metalness Map", "AO Map", "Specular Map",
        "Emissive Map", "Displacement Map"
    };

    auto DnDInputVertical = [&](const char* label, char* buffer, size_t bufSize)
        {
            ImGui::TextUnformatted(label);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            const std::string hidden = std::string("##") + label;
            if (ImGui::InputText(hidden.c_str(), buffer, bufSize))
                Log(std::string("[Edit] ") + label + " = '" + std::string(buffer) + "'");
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
                            SafeCopy(buffer, bufSize, pathUtf8);
                            Log(std::string("[DnD] ") + label + " <- '" + pathUtf8 + "'");
                        }
                        else
                        {
                            Log(std::string("[DnD] Ignored non-file for ") + label);
                        }
                    }
                    else
                    {
                        Log(std::string("[DnD] ParsePayload failed for ") + label);
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::Spacing();
        };

    DnDInputVertical("Mesh Path", meshBuffer, sizeof(meshBuffer));
    for (int i = 0; i < 12; ++i)
        DnDInputVertical(labels[i], textureBuffers[i], sizeof(textureBuffers[i]));

    if (ImGui::Button("Apply Resources"))
    {
        Log("Setting Mesh Path as:" + std::string(meshBuffer));
        SetMeshPath(meshBuffer);
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
        Log("[Apply] Mesh and textures committed to resources");
    }

    {
        float alpha = m_ShaderResources.GetAlphaValue();
        if (ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f))
            m_ShaderResources.SetAlphaValue(alpha);

        bool transparent = IsTransparent();
        if (ImGui::Checkbox("Transparent", &transparent))
            SetTransparent(transparent);
    }

    ImGui::Separator();
    ImGui::Text("Transform & Physics");

    DirectX::XMFLOAT3 pos = m_RigidBody.GetTranslation();
    if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
        m_RigidBody.SetTranslation(pos.x, pos.y, pos.z);

    Quaternion q = m_RigidBody.GetOrientation();
    float orientation[4] = { q.GetI(), q.GetJ(), q.GetK(), q.GetR() };
    if (ImGui::DragFloat4("Orientation (x, y, z, w)", orientation, 0.01f))
        m_RigidBody.SetOrientation({ orientation[3], orientation[0], orientation[1], orientation[2] });

    DirectX::XMFLOAT3 vel;
    XMStoreFloat3(&vel, m_RigidBody.GetVelocity());
    if (ImGui::DragFloat3("Velocity", &vel.x, 0.01f))
        m_RigidBody.SetVelocity({ vel.x, vel.y, vel.z });

    DirectX::XMFLOAT3 acc;
    XMStoreFloat3(&acc, m_RigidBody.GetAcceleration());
    if (ImGui::DragFloat3("Acceleration", &acc.x, 0.01f))
        m_RigidBody.SetAcceleration({ acc.x, acc.y, acc.z });

    DirectX::XMFLOAT3 scale = GetScale();
    if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
        SetScale(scale);

    if (CubeCollider* col = GetCubeCollider())
    {
        DirectX::XMFLOAT3 colScale;
        XMStoreFloat3(&colScale, col->GetScale());
        if (ImGui::DragFloat3("Collider Scale", &colScale.x, 0.01f))
            col->SetScale(XMLoadFloat3(&colScale));

        static const char* stateLabels[] = { "Dynamic", "Static", "Trigger" };
        int stateIndex = static_cast<int>(col->GetColliderState());
        if (ImGui::Combo("Collider State", &stateIndex, stateLabels, IM_ARRAYSIZE(stateLabels)))
            col->SetColliderState(static_cast<ColliderState>(stateIndex));
    }
}

void Mesh::LoadRenderSaveData(const nlohmann::json& json)
{
	IModel::LoadRenderSaveData(json);
	if (!json.is_object()) return;

	auto it = json.find("MeshPath");
	if (it == json.end()) return;

	if (it->is_string())
		m_MeshPath = it->get<std::string>();
	else if (it->is_number_integer())
		m_MeshPath = std::to_string(it->get<long long>());
	else if (it->is_number_unsigned())
		m_MeshPath = std::to_string(it->get<unsigned long long>());
	else if (it->is_number_float())
		m_MeshPath = std::to_string(it->get<double>());
	else if (it->is_boolean())
		m_MeshPath = it->get<bool>() ? "true" : "false";
	else if (it->is_null())
		m_MeshPath.clear();
}

nlohmann::json Mesh::GetRenderSaveData() const
{
	nlohmann::json data = IModel::GetRenderSaveData();
	data["MeshPath"] = m_MeshPath;
	return data;
}

bool Mesh::BuildChild(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	if (m_MeshPath.empty()) return false;
	if (m_Initialized) return true;

	auto sharedMesh = ModelLoader::LoadModel(m_MeshPath);

	if (sharedMesh == nullptr)
	{
		std::string message = "Failed to Load Mesh: " + m_MeshPath;
		LOG_ERROR(message.c_str());

		char buffer[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, buffer);
		LOG_WARNING("Loading From Directory: " + std::string(buffer));

		return false;
	}

	m_MeshBuffer = std::make_unique<StaticVBInstance<MeshBuffer>>(sharedMesh);
	m_MeshBuffer->Init(device);

	m_DebugCube = std::make_unique<ModelCube>();
	m_DebugCube->Build(device, deviceContext);

	m_Initialized = true;
	return true;
}

bool Mesh::RenderChild(ID3D11DeviceContext* deviceContext)
{
	if (!m_Initialized) return false;

	m_MeshBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#ifdef _DEBUG

	// Render Debug Lines
	if (CubeCollider* collider = GetCubeCollider())
	{
		m_WorldMatrixGPU.WorldMatrix = DirectX::XMMatrixTranspose(collider->GetTransformationMatrix());
		m_DebugCube->RenderDebug(deviceContext, m_WorldMatrixGPU);
	}
#endif
	return true;
}

void Mesh::BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//~ Build Shaders
	m_ShaderResources.AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	m_ShaderResources.AddElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources.AddElement("BINORMAL", DXGI_FORMAT_R32G32B32_FLOAT);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = L"Assets/Shader/Shape/CubeShaderVS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources.SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = L"Assets/Shader/Shape/CubeShaderPS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources.SetPixelShaderPath(vertexDesc);

	m_ShaderResources.Build(device, deviceContext);
}

void Mesh::RenderGeometry(ID3D11DeviceContext* deviceContext)
{
	if (!m_Initialized) return;
	m_MeshBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
