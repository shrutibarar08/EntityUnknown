#include "PointLight.h"

#include "Imgui/imgui.h"

#include "Editor/Core/EditorContext.h"

using namespace DirectX;

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
}

void PointLight::SetRange(float range)
{
    m_Range = range;
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
    // TODO: Implement cube map shadowing for point lights.
    // For now, just reset matrices or skip shadow update logic.

    m_ProjMatrix = DirectX::XMMatrixIdentity();
}

void PointLight::RenderControlUI(LevelEditorContext* context)
{
	ImGui::Text("Point Light Settings");
	ImGui::Separator();

	// === Name Edit ===
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
			std::make_unique<CmdRenameLight>(this, GetLightName(), nameBuffer),
			context
		);
	}

	ImGui::Separator();

	// === Ambient Color ===
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