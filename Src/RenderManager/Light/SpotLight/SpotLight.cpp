#include "SpotLight.h"

#include "Imgui/imgui.h"

void SpotLight::SetAmbient(float r, float g, float b, float a)
{
	m_AmbientColor = DirectX::XMFLOAT4(r, g, b, a);
}

void SpotLight::SetDiffuseColor(float r, float g, float b, float a)
{
	m_DiffuseColor = DirectX::XMFLOAT4(r, g, b, a);
}

void SpotLight::SetSpecularColor(float r, float g, float b, float a)
{
	m_SpecularColor = DirectX::XMFLOAT4(r, g, b, a);
}

void SpotLight::SetSpecularPower(float power)
{
	m_SpecularPower = power;
}

void SpotLight::SetPosition(float x, float y, float z)
{
	m_Position = DirectX::XMFLOAT3(x, y, z);
}

void SpotLight::SetDirection(float x, float y, float z)
{
	m_Direction = DirectX::XMFLOAT3(x, y, z);
}

void SpotLight::SetRange(float range)
{
	m_Range = range;
}

void SpotLight::SetSpotAngleDegrees(float degrees)
{
	m_SpotAngleDegree = degrees;
}

DirectX::XMFLOAT4 SpotLight::GetAmbientColor() const
{
	return m_AmbientColor;
}

DirectX::XMFLOAT4 SpotLight::GetDiffuseColor() const
{
	return m_DiffuseColor;
}

DirectX::XMFLOAT4 SpotLight::GetSpecularColor() const
{
	return m_SpecularColor;
}

DirectX::XMFLOAT3 SpotLight::GetPosition() const
{
	return m_Position;
}

DirectX::XMFLOAT3 SpotLight::GetDirection() const
{
	return m_Direction;
}

float SpotLight::GetRange() const
{
	return m_Range;
}

float SpotLight::GetSpotAngleDegree() const
{
	return m_SpotAngleDegree;
}

float SpotLight::GetSpecularPower() const
{
	return m_SpecularPower;
}

SPOT_LIGHT_GPU_DATA SpotLight::GetLightData() const
{
	SPOT_LIGHT_GPU_DATA data{};
	data.SpecularColor = m_SpecularColor;
	data.AmbientColor = m_AmbientColor;
	data.DiffuseColor = m_DiffuseColor;

	data.Position = m_Position;
	data.Range = m_Range;

	data.Direction = m_Direction;

	// Convert degree to cosine(radian) for GPU
	float radians = DirectX::XMConvertToRadians(m_SpotAngleDegree);
	data.SpotAngle = cosf(radians);

	data.SpecularPower = m_SpecularPower;
	data.ViewProjectLightMatrix = GetLightViewProjMatrix();
	return data;
}

DirectX::XMFLOAT3 SpotLight::GetLightPosition() const
{
	return m_Position;
}

void SpotLight::UpdateProjectionMatrix(const Frustum& sceneFrustum)
{
	using namespace DirectX;

	// === Parameters ===
	const float aspectRatio = 1.0f; // symmetric spotlight
	const float nearZ = 0.1f;
	const float farZ = m_Range;

	// Convert the spotlight's half-angle to radians
	const float halfConeAngleRadians = XMConvertToRadians(m_SpotAngleDegree);

	// Build perspective matrix based on cone angle (FOV = full angle)
	m_ProjMatrix = XMMatrixPerspectiveFovLH(halfConeAngleRadians * 2.0f, aspectRatio, nearZ, farZ);
}

void SpotLight::RenderControlUI()
{
	ImGui::Text("Spot Light Settings");
	ImGui::Separator();

	// === Light Name ===
	static char nameBuffer[128]{};
	static uintptr_t lastID = 0;
	uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
	if (lastID != currentID)
	{
		lastID = currentID;
		std::string currentName = GetLightName(); // Ensure returns std::string
		strncpy_s(nameBuffer, currentName.c_str(), sizeof(nameBuffer));
	}
	ImGui::InputText("Light Name", nameBuffer, sizeof(nameBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Rename"))
	{
		SetLightName(nameBuffer);
	}

	ImGui::Separator();

	// === Ambient Color ===
	float ambient[4] = { m_AmbientColor.x, m_AmbientColor.y, m_AmbientColor.z, m_AmbientColor.w };
	if (ImGui::ColorEdit4("Ambient Color", ambient))
		SetAmbient(ambient[0], ambient[1], ambient[2], ambient[3]);

	// === Diffuse Color ===
	float diffuse[4] = { m_DiffuseColor.x, m_DiffuseColor.y, m_DiffuseColor.z, m_DiffuseColor.w };
	if (ImGui::ColorEdit4("Diffuse Color", diffuse))
		SetDiffuseColor(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);

	// === Specular Color ===
	float specular[4] = { m_SpecularColor.x, m_SpecularColor.y, m_SpecularColor.z, m_SpecularColor.w };
	if (ImGui::ColorEdit4("Specular Color", specular))
		SetSpecularColor(specular[0], specular[1], specular[2], specular[3]);

	// === Specular Power ===
	if (ImGui::DragFloat("Specular Power", &m_SpecularPower, 1.0f, 0.0f, 256.0f))
		SetSpecularPower(m_SpecularPower);

	// === Light Range ===
	if (ImGui::DragFloat("Range", &m_Range, 0.1f, 0.5f, 1000.0f))
		SetRange(m_Range);

	// === Spot Angle ===
	if (ImGui::DragFloat("Spot Angle (°)", &m_SpotAngleDegree, 0.1f, 1.0f, 179.0f))
		SetSpotAngleDegrees(m_SpotAngleDegree);

	// === Position ===
	float pos[3] = { m_Position.x, m_Position.y, m_Position.z };
	if (ImGui::DragFloat3("Position", pos, 0.1f))
		SetPosition(pos[0], pos[1], pos[2]);

	// === Direction ===
	float dir[3] = { m_Direction.x, m_Direction.y, m_Direction.z };
	if (ImGui::DragFloat3("Direction", dir, 0.01f))
		SetDirection(dir[0], dir[1], dir[2]);

	ImGui::Separator();
}

