#include "SpotLight.h"
#include "Imgui/imgui.h"
#include "Editor/Core/EditorContext.h"
#include "RenderManager/Model/Cube/ModelCube.h"
#include "RenderManager/RenderQueue/RenderQueue.h"

SpotLight::SpotLight()
{
#ifdef _DEBUG
	m_debugCube = std::make_unique<ModelCube>();
	m_debugCube->GetCubeCollider()->SetScale({ 0.f, 0.0f, 0.0f });
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

SpotLight::~SpotLight()
{
#ifdef _DEBUG
	if (m_debugCube)
	{
		RenderQueue::Get()->RemoveRender(m_debugCube.get());
	}
#endif
}

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
#ifdef _DEBUG
	SyncDebugCubeGizmo();
#endif
}

void SpotLight::SetDirection(float x, float y, float z)
{
	m_Direction = DirectX::XMFLOAT3(x, y, z);
#ifdef _DEBUG
	SyncDebugCubeGizmo();
#endif
}

void SpotLight::SetRange(float range)
{
	m_Range = range;
#ifdef _DEBUG
	SyncDebugCubeGizmo();
#endif
}

void SpotLight::SetSpotAngleDegrees(float degrees)
{
	m_SpotAngleDegree = degrees;
#ifdef _DEBUG
	SyncDebugCubeGizmo();
#endif
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

	const float aspectRatio = 1.0f;
	const float nearZ = 0.1f;
	const float farZ = m_Range;

	const float halfConeAngleRadians = XMConvertToRadians(m_SpotAngleDegree);

	m_ProjMatrix = XMMatrixPerspectiveFovLH(halfConeAngleRadians * 2.0f, aspectRatio, nearZ, farZ);
}

void SpotLight::RenderControlUI(LevelEditorContext* context)
{
	ImGui::Text("Spot Light Settings");
	ImGui::Separator();

	static char nameBuffer[128]{};
	static uintptr_t lastID = 0;
	uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
	if (lastID != currentID)
	{
		lastID = currentID;
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

void SpotLight::LoadLightSaveData(const nlohmann::json& data)
{
	if (data.contains("LightName"))
		m_LightName = data["LightName"].get<std::string>();

	if (data.contains("SpecularPower"))
		m_SpecularPower = data["SpecularPower"].get<float>();

	if (data.contains("Range"))
		m_Range = data["Range"].get<float>();

	if (data.contains("SpotAngleDegree"))
		m_SpotAngleDegree = data["SpotAngleDegree"].get<float>();

	// Colors
	if (data.contains("SpecularColor"))
	{
		const auto& spec = data["SpecularColor"];
		m_SpecularColor = {
			spec.value("x", 0.0f),
			spec.value("y", 0.0f),
			spec.value("z", 0.0f),
			spec.value("w", 1.0f)
		};
	}
	if (data.contains("AmbientColor"))
	{
		const auto& amb = data["AmbientColor"];
		m_AmbientColor = {
			amb.value("x", 0.0f),
			amb.value("y", 0.0f),
			amb.value("z", 0.0f),
			amb.value("w", 1.0f)
		};
	}
	if (data.contains("DiffuseColor"))
	{
		const auto& diff = data["DiffuseColor"];
		m_DiffuseColor = {
			diff.value("x", 0.0f),
			diff.value("y", 0.0f),
			diff.value("z", 0.0f),
			diff.value("w", 1.0f)
		};
	}

	// Position
	if (data.contains("Position"))
	{
		const auto& pos = data["Position"];
		m_Position = {
			pos.value("x", 0.0f),
			pos.value("y", 0.0f),
			pos.value("z", 0.0f)
		};
	}

	// Direction
	if (data.contains("Direction"))
	{
		const auto& dir = data["Direction"];
		m_Direction = {
			dir.value("x", 0.0f),
			dir.value("y", 0.0f),
			dir.value("z", 1.0f)
		};
	}
	SyncDebugCubeGizmo();
}

nlohmann::json SpotLight::GetLightSaveData() const
{
	nlohmann::json data;

	data["LightName"] = m_LightName;

	data["SpecularPower"] = m_SpecularPower;
	data["Range"] = m_Range;
	data["SpotAngleDegree"] = m_SpotAngleDegree;

	// Colors
	data["SpecularColor"] =
	{
		{ "x", m_SpecularColor.x },
		{ "y", m_SpecularColor.y },
		{ "z", m_SpecularColor.z },
		{ "w", m_SpecularColor.w }
	};
	data["AmbientColor"] =
	{
		{ "x", m_AmbientColor.x },
		{ "y", m_AmbientColor.y },
		{ "z", m_AmbientColor.z },
		{ "w", m_AmbientColor.w }
	};
	data["DiffuseColor"] =
	{
		{ "x", m_DiffuseColor.x },
		{ "y", m_DiffuseColor.y },
		{ "z", m_DiffuseColor.z },
		{ "w", m_DiffuseColor.w }
	};

	// Position
	data["Position"] =
	{
		{ "x", m_Position.x },
		{ "y", m_Position.y },
		{ "z", m_Position.z }
	};

	// Direction
	data["Direction"] =
	{
		{ "x", m_Direction.x },
		{ "y", m_Direction.y },
		{ "z", m_Direction.z }
	};

	return data;
}

void SpotLight::SyncDebugCubeGizmo()
{
#ifdef _DEBUG
	if (!m_debugCube) return;

	auto* rb = m_debugCube->GetCubeCollider()->GetRigidBody();

	rb->SetTranslation(m_Position);

	DirectX::XMVECTOR to = DirectX::XMLoadFloat3(&m_Direction);
	const float len2 = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(to));
	if (len2 < 1e-12f) 
	{
		to = DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f);
	}
	else 
	{
		to = DirectX::XMVectorScale(to, 1.0f / sqrtf(len2));
	}

	const DirectX::XMVECTOR from = DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f);
	DirectX::XMVECTOR q = SafeFromToQuat(from, to);
	Quaternion qq(DirectX::XMVectorGetW(q), DirectX::XMVectorGetX(q), DirectX::XMVectorGetY(q), DirectX::XMVectorGetZ(q));
	rb->SetOrientation(qq);

	const float halfAngleRad = DirectX::XMConvertToRadians(m_SpotAngleDegree * 0.5f);

	const float len = std::max(0.25f, m_Range * 0.1f);

	const float radius = std::max(0.05f, tanf(halfAngleRad) * len);

	m_debugCube->SetScaleX(len);
	m_debugCube->SetScaleY(radius);
	m_debugCube->SetScaleZ(radius);
#endif
}
