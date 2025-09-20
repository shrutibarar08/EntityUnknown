#include "DirectionalLight.h"

#include "Imgui/imgui.h"
#include "Utils/Logger/Logger.h"

#include "Editor/Core/EditorContext.h"

#include "RenderManager/Model/Cube/ModelCube.h"
#include "RenderManager/RenderQueue/RenderQueue.h"

DirectionalLight::DirectionalLight()
{
#ifdef _DEBUG
	m_debugCube = std::make_unique<ModelCube>();
	m_debugCube->GetCubeCollider()->SetScale({ 0.0f, 0.0f, 0.0f });
	m_debugCube->SetScaleX(0.25f);
	m_debugCube->SetScaleY(0.40f);
	m_debugCube->SetScaleZ(0.25f);
	m_debugCube->SetTransparent(true);
	m_debugCube->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_debugCube->GetShaderResource()->SetTexture("Assets/Texture/debug-image.jpg");
	m_debugCube->SetDebugOnly(true);
	RenderQueue::Get()->AddRender(m_debugCube.get());
	SyncDebugCubeOrientation();
#endif
}

DirectionalLight::~DirectionalLight()
{
#ifdef _DEBUG
	if (m_debugCube)
	{
		RenderQueue::Get()->RemoveRender(m_debugCube.get());
	}
#endif
}

void DirectionalLight::SetAmbient(float red, float green, float blue, float alpha)
{
	m_AmbientColor = DirectX::XMFLOAT4(red, green, blue, alpha);
}

DirectX::XMFLOAT4 DirectionalLight::GetAmbientColor() const
{
	return m_AmbientColor;
}

void DirectionalLight::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_DiffuseColor = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void DirectionalLight::SetDirection(float x, float y, float z)
{
	m_Direction = DirectX::XMFLOAT3(x, y, z);
#ifdef _DEBUG
	SyncDebugCubeOrientation();
#endif
}

void DirectionalLight::SetSpecularColor(float red, float green, float blue, float alpha)
{
	m_SpecularColor = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void DirectionalLight::SetSpecularPower(float power)
{
	m_SpecularPower = power;
}

float DirectionalLight::GetSpecularPower() const
{
	return m_SpecularPower;
}

DIRECTIONAL_LIGHT_GPU_DATA DirectionalLight::GetLightData() const
{
	DIRECTIONAL_LIGHT_GPU_DATA data{};
	data.SpecularColor = m_SpecularColor;
	data.AmbientColor = m_AmbientColor;
	data.DiffuseColor = m_DiffuseColor;
	data.Direction = m_Direction;
	data.SpecularPower = m_SpecularPower;
	data.ViewProjectLightMatrix = GetLightViewProjMatrix();
	return data;
}

DirectX::XMFLOAT3 DirectionalLight::GetLightPosition() const
{
	//~ don't need to respond with distance.
	return { -10, -10, -10 };
}

void DirectionalLight::UpdateProjectionMatrix(const Frustum& sceneFrustum)
{
	using namespace DirectX;

	XMFLOAT3 center = sceneFrustum.GetCenter();
	XMFLOAT3 extents = sceneFrustum.GetExtents(); // Half-size in each axis

	// Transform frustum corners to light view space
	XMMATRIX view = m_ViewMatrix; // Already set
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	// Define 8 corners of the scene frustum in world space
	std::array<XMFLOAT3, 8> corners = sceneFrustum.GetCorners();
	std::array<XMVECTOR, 8> lightSpaceCorners;

	for (int i = 0; i < 8; ++i)
	{
		XMVECTOR worldPos = XMLoadFloat3(&corners[i]);
		lightSpaceCorners[i] = XMVector3Transform(worldPos, view);
	}

	// Find min/max in light view space
	XMVECTOR min = lightSpaceCorners[0];
	XMVECTOR max = lightSpaceCorners[0];

	for (int i = 1; i < 8; ++i)
	{
		min = XMVectorMin(min, lightSpaceCorners[i]);
		max = XMVectorMax(max, lightSpaceCorners[i]);
	}

	XMFLOAT3 min3, max3;
	XMStoreFloat3(&min3, min);
	XMStoreFloat3(&max3, max);

	// Now compute orthographic matrix that bounds the scene in light-space
	m_ProjMatrix = XMMatrixOrthographicOffCenterLH(
		min3.x, max3.x,
		min3.y, max3.y,
		min3.z, max3.z
	);
}

void DirectionalLight::RenderControlUI(LevelEditorContext* context)
{
	ImGui::Text("Directional Light Settings");
	ImGui::Separator();

	//=== Name Edit ===
		static char nameBuffer[128]{};
	static uintptr_t lastObjectID = 0;
	uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
	if (lastObjectID != currentID)
	{
		lastObjectID = currentID;
		std::string currentName = GetLightName(); // Assuming returns std::string
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

	// === Direction ===
	float direction[3] = { m_Direction.x, m_Direction.y, m_Direction.z };
	if (ImGui::DragFloat3("Direction", direction, 0.01f, -1.0f, 1.0f))
		SetDirection(direction[0], direction[1], direction[2]);

	ImGui::Separator();
}

void DirectionalLight::LoadLightSaveData(const nlohmann::json& data)
{
	if (data.contains("LightName"))
		m_LightName = data["LightName"].get<std::string>();

	if (data.contains("SpecularPower"))
		m_SpecularPower = data["SpecularPower"].get<float>();

	// Colors
	if (data.contains("SpecularColor"))
	{
		const auto& spec = data["SpecularColor"];
		m_SpecularColor =
		{
			spec.value("x", 0.0f),
			spec.value("y", 0.0f),
			spec.value("z", 0.0f),
			spec.value("w", 1.0f)
		};
	}
	if (data.contains("AmbientColor")) 
	{
		const auto& amb = data["AmbientColor"];
		m_AmbientColor = 
		{
			amb.value("x", 0.0f),
			amb.value("y", 0.0f),
			amb.value("z", 0.0f),
			amb.value("w", 1.0f)
		};
	}
	if (data.contains("DiffuseColor")) 
	{
		const auto& diff = data["DiffuseColor"];
		m_DiffuseColor = 
		{
			diff.value("x", 0.0f),
			diff.value("y", 0.0f),
			diff.value("z", 0.0f),
			diff.value("w", 1.0f)
		};
	}

	// Direction
	if (data.contains("Direction")) 
	{
		const auto& dir = data["Direction"];
		m_Direction = 
		{
			dir.value("x", 0.0f),
			dir.value("y", 0.0f),
			dir.value("z", 1.0f) // default looking "forward"
		};
	}
	SyncDebugCubeOrientation();
}

nlohmann::json DirectionalLight::GetLightSaveData() const
{
	nlohmann::json data;

	data["LightName"] = m_LightName;
	data["SpecularPower"] = m_SpecularPower;

	// SpecularColor
	data["SpecularColor"] = 
	{
		{ "x", m_SpecularColor.x },
		{ "y", m_SpecularColor.y },
		{ "z", m_SpecularColor.z },
		{ "w", m_SpecularColor.w }
	};

	// AmbientColor
	data["AmbientColor"] =
	{
		{ "x", m_AmbientColor.x },
		{ "y", m_AmbientColor.y },
		{ "z", m_AmbientColor.z },
		{ "w", m_AmbientColor.w }
	};

	// DiffuseColor
	data["DiffuseColor"] =
	{
		{ "x", m_DiffuseColor.x },
		{ "y", m_DiffuseColor.y },
		{ "z", m_DiffuseColor.z },
		{ "w", m_DiffuseColor.w }
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

void DirectionalLight::SyncDebugCubeOrientation()
{
#ifdef _DEBUG
	if (!m_debugCube) return;

	const DirectX::XMVECTOR from = DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f);
	DirectX::XMVECTOR to = XMLoadFloat3(&m_Direction);

	DirectX::XMVECTOR q = SafeFromToQuat(from, to);

	// Your Quaternion ctor appears to be (w, x, y, z)
	Quaternion qq(DirectX::XMVectorGetW(q), DirectX::XMVectorGetX(q), DirectX::XMVectorGetY(q), DirectX::XMVectorGetZ(q));
	m_debugCube->GetCubeCollider()->GetRigidBody()->SetOrientation(qq);
#endif
}

DirectX::XMFLOAT4 DirectionalLight::GetSpecularColor() const
{
	return m_SpecularColor;
}

DirectX::XMFLOAT4 DirectionalLight::GetDiffuseColor() const
{
	return m_DiffuseColor;
}

DirectX::XMFLOAT3 DirectionalLight::GetDirection() const
{
	return m_Direction;
}
