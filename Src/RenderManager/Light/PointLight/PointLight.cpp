#include "PointLight.h"

#include "Imgui/imgui.h"

#include "Editor/Core/EditorContext.h"
#include "RenderManager/Model/Cube/ModelCube.h"
#include "RenderManager/RenderQueue/RenderQueue.h"

using namespace DirectX;

PointLight::PointLight()
{
#ifdef _DEBUG
    m_debugCube = std::make_unique<ModelCube>();
    m_debugCube->GetCubeCollider()->SetScale({ 0.0f, 0.0f, 0.0f });
    m_debugCube->SetScaleX(0.25f);
    m_debugCube->SetScaleY(0.25f);
    m_debugCube->SetScaleZ(0.25f);
    m_debugCube->SetTransparent(true);
    m_debugCube->SetDebugOnly(true);
    m_debugCube->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_debugCube->GetShaderResource()->SetTexture("Assets/Texture/debug-image.jpg");
    RenderQueue::Get()->AddRender(m_debugCube.get());

    SyncDebugCubeGizmo();
#endif
}

PointLight::~PointLight()
{
#ifdef _DEBUG
    if (m_debugCube)
    {
        RenderQueue::Get()->RemoveRender(m_debugCube.get());
    }
#endif
}

void PointLight::SetAmbient(float r, float g, float b, float a)
{
    m_AmbientColor = XMFLOAT4(r, g, b, a);
}

void PointLight::SetDiffuseColor(float r, float g, float b, float a)
{
    m_DiffuseColor = XMFLOAT4(r, g, b, a);
}

void PointLight::SetSpecularColor(float r, float g, float b, float a)
{
    m_SpecularColor = XMFLOAT4(r, g, b, a);
}

void PointLight::SetSpecularPower(float power)
{
    m_SpecularPower = power;
}

void PointLight::SetPosition(float x, float y, float z)
{
    m_Position = XMFLOAT3(x, y, z);
#ifdef _DEBUG
    SyncDebugCubeGizmo();
#endif
}

void PointLight::SetRange(float range)
{
    m_Range = range;
#ifdef _DEBUG
    SyncDebugCubeGizmo();
#endif
}

XMFLOAT4 PointLight::GetAmbientColor() const
{
    return m_AmbientColor;
}

XMFLOAT4 PointLight::GetDiffuseColor() const
{
    return m_DiffuseColor;
}

XMFLOAT4 PointLight::GetSpecularColor() const
{
    return m_SpecularColor;
}

XMFLOAT3 PointLight::GetPosition() const
{
    return m_Position;
}

float PointLight::GetRange() const
{
    return m_Range;
}

float PointLight::GetSpecularPower() const
{
    return m_SpecularPower;
}

POINT_LIGHT_GPU_DATA PointLight::GetLightData() const
{
    POINT_LIGHT_GPU_DATA data{};
    data.SpecularColor = m_SpecularColor;
    data.AmbientColor = m_AmbientColor;
    data.DiffuseColor = m_DiffuseColor;
    data.Position = m_Position;
    data.Range = m_Range;
    data.SpecularPower = m_SpecularPower;
    data.ViewProjectLightMatrix = GetLightViewProjMatrix();
    return data;
}

XMFLOAT3 PointLight::GetLightPosition() const
{
    return m_Position;
}

void PointLight::UpdateProjectionMatrix(const Frustum& sceneFrustum)
{
    m_ProjMatrix = DirectX::XMMatrixIdentity();
}

void PointLight::RenderControlUI(LevelEditorContext* context)
{
    ImGui::Text("Point Light Settings");
    ImGui::Separator();

    static char nameBuffer[128]{};
    static uintptr_t lastObjectID = 0;
    uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
    if (lastObjectID != currentID)
    {
        lastObjectID = currentID;
        std::string currentName = GetLightName();
        strncpy_s(nameBuffer, currentName.c_str(), sizeof(nameBuffer));
    }
    ImGui::InputText("Light Name", nameBuffer, sizeof(nameBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Rename"))
    {
        context->GetCommandStack()->Execute(
            std::make_unique<CmdRenameLight>(this, nameBuffer),
            context
        );
    }

    ImGui::Separator();

    ImGui::Text("Ambient Color");
    float ambient[4] = { m_AmbientColor.x, m_AmbientColor.y, m_AmbientColor.z, m_AmbientColor.w };
    if (ImGui::ColorEdit4("Ambient", ambient))
        SetAmbient(ambient[0], ambient[1], ambient[2], ambient[3]);

    // === Diffuse Color ===
    ImGui::Text("Diffuse Color");
    float diffuse[4] = { m_DiffuseColor.x, m_DiffuseColor.y, m_DiffuseColor.z, m_DiffuseColor.w };
    if (ImGui::ColorEdit4("Diffuse", diffuse))
        SetDiffuseColor(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);

    // === Specular Color ===
    ImGui::Text("Specular Color");
    float specular[4] = { m_SpecularColor.x, m_SpecularColor.y, m_SpecularColor.z, m_SpecularColor.w };
    if (ImGui::ColorEdit4("Specular", specular))
        SetSpecularColor(specular[0], specular[1], specular[2], specular[3]);

    // === Specular Power ===
    if (ImGui::DragFloat("Specular Power", &m_SpecularPower, 1.0f, 0.0f, 256.0f))
        SetSpecularPower(m_SpecularPower);

    // === Light Range ===
    if (ImGui::DragFloat("Range", &m_Range, 0.1f, 0.5f, 1000.0f))
        SetRange(m_Range);

    // === Position ===
    float position[3] = { m_Position.x, m_Position.y, m_Position.z };
    if (ImGui::DragFloat3("Position", position, 0.1f))
        SetPosition(position[0], position[1], position[2]);

    ImGui::Separator();
}

void PointLight::LoadLightSaveData(const nlohmann::json& data)
{
    if (data.contains("LightName"))
        m_LightName = data["LightName"].get<std::string>();

    if (data.contains("SpecularPower"))
        m_SpecularPower = data["SpecularPower"].get<float>();

    if (data.contains("Range"))
        m_Range = data["Range"].get<float>();

    // Colors
    if (data.contains("SpecularColor")) {
        const auto& spec = data["SpecularColor"];
        m_SpecularColor = {
            spec.value("x", 0.0f),
            spec.value("y", 0.0f),
            spec.value("z", 0.0f),
            spec.value("w", 1.0f)
        };
    }
    if (data.contains("AmbientColor")) {
        const auto& amb = data["AmbientColor"];
        m_AmbientColor = {
            amb.value("x", 0.0f),
            amb.value("y", 0.0f),
            amb.value("z", 0.0f),
            amb.value("w", 1.0f)
        };
    }
    if (data.contains("DiffuseColor")) {
        const auto& diff = data["DiffuseColor"];
        m_DiffuseColor = {
            diff.value("x", 0.0f),
            diff.value("y", 0.0f),
            diff.value("z", 0.0f),
            diff.value("w", 1.0f)
        };
    }

    // Position
    if (data.contains("Position")) {
        const auto& pos = data["Position"];
        m_Position = {
            pos.value("x", 0.0f),
            pos.value("y", 0.0f),
            pos.value("z", 0.0f)
        };
    }
    SyncDebugCubeGizmo();
}

nlohmann::json PointLight::GetLightSaveData() const
{
    nlohmann::json data;

    data["LightName"] = m_LightName;
    data["SpecularPower"] = m_SpecularPower;
    data["Range"] = m_Range;

    // SpecularColor
    data["SpecularColor"] = {
        { "x", m_SpecularColor.x },
        { "y", m_SpecularColor.y },
        { "z", m_SpecularColor.z },
        { "w", m_SpecularColor.w }
    };

    // AmbientColor
    data["AmbientColor"] = {
        { "x", m_AmbientColor.x },
        { "y", m_AmbientColor.y },
        { "z", m_AmbientColor.z },
        { "w", m_AmbientColor.w }
    };

    // DiffuseColor
    data["DiffuseColor"] = {
        { "x", m_DiffuseColor.x },
        { "y", m_DiffuseColor.y },
        { "z", m_DiffuseColor.z },
        { "w", m_DiffuseColor.w }
    };

    // Position
    data["Position"] = {
        { "x", m_Position.x },
        { "y", m_Position.y },
        { "z", m_Position.z }
    };

    return data;
}

void PointLight::SyncDebugCubeGizmo()
{
#ifdef _DEBUG
    if (!m_debugCube) return;

    auto* rb = m_debugCube->GetCubeCollider()->GetRigidBody();
    rb->SetTranslation(m_Position);

    const XMVECTOR from = XMVectorSet(1.f, 0.f, 0.f, 0.f);
    const XMVECTOR to = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    XMVECTOR q = SafeFromToQuat(from, to);
    Quaternion qq(XMVectorGetW(q), XMVectorGetX(q), XMVectorGetY(q), XMVectorGetZ(q));
    rb->SetOrientation(qq);

    const float len = std::max(0.25f, m_Range * 0.1f);
    m_debugCube->SetScaleX(len);
    m_debugCube->SetScaleY(0.25f);
    m_debugCube->SetScaleZ(0.25f);
#endif
}
