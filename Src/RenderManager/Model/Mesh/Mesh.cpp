#include "Mesh.h"

#include "ExceptionManager/IException.h"
#include "Imgui/imgui.h"
#include "RenderManager/Model/ModelLoader/ObjLoader/ObjLoader.h"

#include "Editor/Core/UiPolicy/WidgetPolicy/ContentBrowser/ImGuiContentBrowserPolicy.h"
#include "Editor/EditorState.h"

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
    if (ImGui::CollapsingHeader("Object & Render", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static char nameBuffer[128]{};
        static uintptr_t lastObjectID = 0;
        const uintptr_t currentID = reinterpret_cast<uintptr_t>(this);

        if (lastObjectID != currentID)
        {
            lastObjectID = currentID;
            UI_SafeCopy(nameBuffer, sizeof(nameBuffer), GetName());
            UIHelpers::g_PathBuffers.clear();
        }

        if (ImGui::InputText(UI_ObjectRenameLabel(), nameBuffer, sizeof(nameBuffer)))
        {}
        ImGui::SameLine();
        if (ImGui::Button("Rename"))
            SetName(std::string(nameBuffer));

        ImGui::Separator();

        UI_PathFieldWithApplyAndDnD(
            "Mesh Path",
            m_MeshPath,
            [&](const std::string& p) { SetMeshPath(p); },
            false
        );

        ImGui::Separator();

        {
            const char* topoLabels[] = { "Triangle List", "Triangle Strip", "Line List", "Line Strip", "Point List" };
            int topoIndex = UI_TopologyToIndex(m_PrimitiveTopology);
            if (ImGui::Combo("Primitive Topology", &topoIndex, topoLabels, IM_ARRAYSIZE(topoLabels)))
            {
                m_PrimitiveTopology = UI_IndexToTopology(topoIndex);
            }
        }

        ImGui::Separator();

        {
            auto& shader = m_ShaderResources;

            float alpha = shader.GetAlphaValue();
            if (ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f))
                shader.SetAlphaValue(alpha);

            bool transparent = IsTransparent();
            if (ImGui::Checkbox("Transparent", &transparent))
                SetTransparent(transparent);
        }

        ImGui::Separator();

        {
            static int xMul = 1;
            static int yMul = 1;

            ImGui::DragInt("Tile X", &xMul, 0.1f, 1, 64);
            ImGui::DragInt("Tile Y", &yMul, 0.1f, 1, 64);

            if (ImGui::Button("Apply Texture Multiplier"))
                SetTextureMultiplier(xMul, yMul);
        }
    }

    UI_Section_TransformAndPhysics(context);
    UI_Section_Textures(context);
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

	m_MeshBuffer->Render(deviceContext, m_PrimitiveTopology);

#ifdef _DEBUG
	if (!EDITOR_STATE::PLAY_STATE)
	{
		// Render Debug Lines
		if (CubeCollider* collider = GetCubeCollider())
		{
			m_WorldMatrixGPU.WorldMatrix = DirectX::XMMatrixTranspose(collider->GetTransformationMatrix());
			m_DebugCube->RenderDebug(deviceContext, m_WorldMatrixGPU);
		}
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

	EU_BLOB_INIT_DESC vertexDesc{};
	vertexDesc.FilePath = L"Assets/Shader/Shape/CubeShaderVS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources.SetVertexShaderPath(vertexDesc);

	EU_BLOB_INIT_DESC PixelDesc{};
	vertexDesc.FilePath = L"Assets/Shader/Shape/CubeShaderPS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources.SetPixelShaderPath(vertexDesc);

	m_ShaderResources.Build(device, deviceContext);
}

void Mesh::RenderGeometry(ID3D11DeviceContext* deviceContext)
{
	if (!m_Initialized) return;
	m_MeshBuffer->Render(deviceContext, m_PrimitiveTopology);
}
