#include "IRender.h"

#include <format>
#include "Imgui/imgui.h"

#include "Editor/Core/UiPolicy/WidgetPolicy/ContentBrowser/ImGuiContentBrowserPolicy.h"
#include "Editor/EditorState.h"

IRender::IRender()
{
	m_CubeCollider = std::make_unique<CubeCollider>(&m_RigidBody);
	m_bDirty = true;
}

bool IRender::Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	if (!m_bCommonDataInitialized)
	{
		m_bCommonDataInitialized = true;
		m_VertexMetadataCB = std::make_unique<ConstantBuffer<VERTEX_BUFFER_METADATA_GPU>>(device);
		m_PixelMetadataCB = std::make_unique<ConstantBuffer<PIXEL_BUFFER_METADATA_GPU>>(device);
	}

	m_ShaderResources.ClearInputElements();
	BuildShaders(device, deviceContext);
	m_LightManager.Build(device);

	return true;
}

bool IRender::Render(ID3D11DeviceContext* deviceContext)
{
	if (!m_bCommonDataInitialized) return false;
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return true;

	UpdateVertexMetaDataConstantBuffer(deviceContext);
	BindVertexMetaDataConstantBuffer(deviceContext);

	UpdatePixelMetaDataConstantBuffer(deviceContext);
	BindPixelMetaDataConstantBuffer(deviceContext);

	if (m_LightEnabled)
	{
		m_LightManager.Update(deviceContext, m_RigidBody.GetPosition());
		m_LightManager.Bind(deviceContext);
	}
	m_ShaderResources.Render(deviceContext);
	return true;
}

bool IRender::RenderDepthOnly(
	ID3D11DeviceContext* deviceContext,
	const DirectX::XMMATRIX& lightViewMatrix,
	const DirectX::XMMATRIX& ProjectionMatrix)
{
	if (!m_bCommonDataInitialized) return false;
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return true;

	m_LightManager.Update(deviceContext, m_RigidBody.GetPosition());

	VERTEX_BUFFER_METADATA_GPU data = m_WorldMatrixGPU;
	data.ProjectionMatrix = DirectX::XMMatrixTranspose(ProjectionMatrix);
	data.ViewMatrix = DirectX::XMMatrixTranspose(lightViewMatrix);
	
	UpdateVertexMetaDataConstantBuffer(deviceContext, data);
	BindVertexMetaDataConstantBuffer(deviceContext);
	deviceContext->PSSetShader(nullptr, nullptr, 0u);

	m_ShaderResources.RenderVertexShader(deviceContext);
	RenderGeometry(deviceContext);
	return true;
}

bool IRender::UnBind(ID3D11DeviceContext* deviceContext)
{
	if (!m_bCommonDataInitialized) return false;
	if (m_LightEnabled)
	{
		m_LightManager.UnBind(deviceContext);
	}
	return true;
}

void IRender::RenderControlUI(LevelEditorContext* context)
{
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return;

	UI_Section_ObjectAndRender(context);
	UI_Section_TransformAndPhysics(context);
	UI_Section_Textures(context);
}

void IRender::SetScreenWidth(int width)
{
	if (m_ScreenWidth != width) m_bDirty = true;

	m_ScreenWidth = width;
}

void IRender::SetScreenHeight(int height)
{
	if (m_ScreenHeight != height) m_bDirty = true;

	m_ScreenHeight = height;
}

void IRender::SetDirty(bool flag)
{
	m_bDirty = flag;
}

bool IRender::IsDirty() const
{
	return m_bDirty;
}

void IRender::LoadRenderSaveData(const nlohmann::json& j)
{
	// tiny helpers
	auto get_float = [](const nlohmann::json& n, const char* key, float def) -> float {
		if (!n.contains(key)) return def;
		const auto& v = n.at(key);
		if (v.is_number_float() || v.is_number_integer()) return v.get<float>();
		if (v.is_string()) { try { return std::stof(v.get<std::string>()); } catch (...) {} }
		return def;
		};
	auto get_int = [](const nlohmann::json& n, const char* key, int def) -> int {
		if (!n.contains(key)) return def;
		const auto& v = n.at(key);
		if (v.is_number_integer()) return v.get<int>();
		if (v.is_number_float())  return static_cast<int>(v.get<float>());
		if (v.is_string()) { try { return std::stoi(v.get<std::string>()); } catch (...) {} }
		return def;
		};
	auto get_bool = [](const nlohmann::json& n, const char* key, bool def) -> bool {
		if (!n.contains(key)) return def;
		const auto& v = n.at(key);
		if (v.is_boolean()) return v.get<bool>();
		if (v.is_number_integer()) return v.get<int>() != 0;
		if (v.is_string()) {
			std::string s = v.get<std::string>();
			for (auto& c : s) c = (char)std::tolower(c);
			if (s == "true" || s == "1" || s == "yes") return true;
			if (s == "false" || s == "0" || s == "no")  return false;
		}
		return def;
		};
	auto vec3_from = [&](const nlohmann::json& n, const char* key, DirectX::XMFLOAT3 def = { 0,0,0 }) -> DirectX::XMFLOAT3 {
		if (!n.contains(key) || !n.at(key).is_object()) return def;
		const auto& o = n.at(key);
		return DirectX::XMFLOAT3{
			get_float(o, "x", def.x),
			get_float(o, "y", def.y),
			get_float(o, "z", def.z)
		};
		};

	// === Name ===
	if (j.contains("Name") && j["Name"].is_string())
		m_Name = j["Name"].get<std::string>();

	// === TextureMultiplier ===
	if (j.contains("TextureMultiplier") && j["TextureMultiplier"].is_object()) {
		const auto& tm = j["TextureMultiplier"];
		m_TextureMultiplierX = get_int(tm, "x", m_TextureMultiplierX);
		m_TextureMultiplierY = get_int(tm, "y", m_TextureMultiplierY);
	}

	// === Transparent ===
	if (j.contains("Transparent"))
		SetTransparent(get_bool(j, "Transparent", m_bTransparent));

	// === AlphaValue ===
	if (j.contains("AlphaValue")) {
		float a = get_float(j, "AlphaValue", m_ShaderResources.GetAlphaValue());
		m_ShaderResources.SetAlphaValue(a);
		if (a < 1.0f) SetTransparent(true);
	}

	// === Transform ===
	if (j.contains("Transform") && j["Transform"].is_object()) {
		const auto& t = j["Transform"];

		// Position
		{
			DirectX::XMFLOAT3 pos = vec3_from(t, "Position");
			m_RigidBody.SetTranslation(pos.x, pos.y, pos.z);
		}

		// Orientation (Quaternion w,x,y,z; JSON stores x=i,y=j,z=k,w=r)
		if (t.contains("Orientation") && t["Orientation"].is_object()) {
			const auto& o = t["Orientation"];
			float x = get_float(o, "x", 0.0f);
			float y = get_float(o, "y", 0.0f);
			float z = get_float(o, "z", 0.0f);
			float w = get_float(o, "w", 1.0f);
			m_RigidBody.SetOrientation(Quaternion(w, x, y, z));
		}

		// Scale
		{
			DirectX::XMFLOAT3 sc = vec3_from(t, "Scale", GetScale());
			SetScale(sc.x, sc.y, sc.z);
		}
	}

	// === Physics ===
	if (j.contains("Physics") && j["Physics"].is_object()) {
		const auto& p = j["Physics"];

		// Velocity
		if (p.contains("Velocity") && p["Velocity"].is_object()) {
			DirectX::XMFLOAT3 v = vec3_from(p, "Velocity");
			m_RigidBody.SetVelocity(DirectX::XMVectorSet(v.x, v.y, v.z, 0.0f));
		}

		// Acceleration
		if (p.contains("Acceleration") && p["Acceleration"].is_object()) {
			DirectX::XMFLOAT3 a = vec3_from(p, "Acceleration");
			m_RigidBody.SetAcceleration(DirectX::XMVectorSet(a.x, a.y, a.z, 0.0f));
		}

		// Collider (CubeCollider only if present)
		if (p.contains("Collider") && p["Collider"].is_object()) {
			if (CubeCollider* collider = GetCubeCollider()) {
				const auto& c = p["Collider"];

				// Scale
				if (c.contains("Scale") && c["Scale"].is_object()) {
					DirectX::XMFLOAT3 s = vec3_from(c, "Scale");
					collider->SetScale(DirectX::XMVectorSet(s.x, s.y, s.z, 0.0f));
				}

				// State
				if (c.contains("State") && c["State"].is_string()) {
					const std::string st = c["State"].get<std::string>();
					if (st == "Dynamic") collider->SetColliderState(ColliderState::Dynamic);
					else if (st == "Static")  collider->SetColliderState(ColliderState::Static);
					else if (st == "Trigger") collider->SetColliderState(ColliderState::Trigger);
				}
			}
		}
	}

	// === Shader Resources ===
	if (j.contains("Shader") && j["Shader"].is_object()) {
		const auto& s = j["Shader"];

		auto set_if_str = [&](const char* key, auto setter) {
			if (s.contains(key) && s.at(key).is_string()) {
				const std::string& path = s.at(key).get<std::string>();
				if (!path.empty()) setter(path);
			}
			};

		auto& sr = m_ShaderResources;
		set_if_str("Texture", [&](auto& v) { sr.SetTexture(v); });
		set_if_str("Secondary Texture", [&](auto& v) { sr.SetSecondaryTexture(v); });
		set_if_str("Normal Map", [&](auto& v) { sr.SetNormalMap(v); });
		set_if_str("Alpha Map", [&](auto& v) { sr.SetAlphaMap(v); });
		set_if_str("Height Map", [&](auto& v) { sr.SetHeightMap(v); });
		set_if_str("AO Map", [&](auto& v) { sr.SetAOMap(v); });
		set_if_str("Specular Map", [&](auto& v) { sr.SetSpecularMap(v); });
		set_if_str("Emissive Map", [&](auto& v) { sr.SetEmissiveMap(v); });
		set_if_str("Light Map", [&](auto& v) { sr.SetLightMap(v); });
		set_if_str("Metalness Map", [&](auto& v) { sr.SetMetalnessMap(v); });
		set_if_str("Roughness Map", [&](auto& v) { sr.SetRoughnessMap(v); });
		set_if_str("Displacement Map", [&](auto& v) { sr.SetDisplacementMap(v); });
	}
}

nlohmann::json IRender::GetRenderSaveData() const
{
	nlohmann::json out = nlohmann::json::object();

	// === Name / Type / Alpha / Transparent / TextureMultiplier ===
	out["Name"] = m_Name;
	out["Type"] = GetTypeName();
	out["AlphaValue"] = m_ShaderResources.GetAlphaValue(); // numeric (not string)
	out["Transparent"] = m_bTransparent;

	out["TextureMultiplier"] = {
		{ "x", m_TextureMultiplierX },
		{ "y", m_TextureMultiplierY }
	};

	// === Transform ===
	{
		// Position
		DirectX::XMFLOAT3 pos = m_RigidBody.GetTranslation();
		nlohmann::json jPos = { {"x", pos.x}, {"y", pos.y}, {"z", pos.z} };

		// Orientation (x=i, y=j, z=k, w=r)
		Quaternion q = m_RigidBody.GetOrientation();
		nlohmann::json jOri = { {"x", q.GetI()}, {"y", q.GetJ()}, {"z", q.GetK()}, {"w", q.GetR()} };

		// Scale
		DirectX::XMFLOAT3 sc = GetScale();
		nlohmann::json jSc = { {"x", sc.x}, {"y", sc.y}, {"z", sc.z} };

		out["Transform"] = {
			{ "Position",    std::move(jPos) },
			{ "Orientation", std::move(jOri) },
			{ "Scale",       std::move(jSc)  }
		};
	}

	// === Physics ===
	{
		// Velocity
		DirectX::XMFLOAT3 vel{};
		XMStoreFloat3(&vel, m_RigidBody.GetVelocity());
		nlohmann::json jVel = { {"x", vel.x}, {"y", vel.y}, {"z", vel.z} };

		// Acceleration
		DirectX::XMFLOAT3 acc{};
		XMStoreFloat3(&acc, m_RigidBody.GetAcceleration());
		nlohmann::json jAcc = { {"x", acc.x}, {"y", acc.y}, {"z", acc.z} };

		nlohmann::json jPhys = {
			{ "Velocity",     std::move(jVel) },
			{ "Acceleration", std::move(jAcc) }
		};

		// Collider (CubeCollider only if present)
		if (CubeCollider* collider = GetCubeCollider()) {
			DirectX::XMFLOAT3 csc{};
			XMStoreFloat3(&csc, collider->GetScale());
			nlohmann::json jScale = { {"x", csc.x}, {"y", csc.y}, {"z", csc.z} };

			const char* stateStr = "Unknown";
			switch (collider->GetColliderState()) {
			case ColliderState::Dynamic: stateStr = "Dynamic"; break;
			case ColliderState::Static:  stateStr = "Static";  break;
			case ColliderState::Trigger: stateStr = "Trigger"; break;
			default: break;
			}

			jPhys["Collider"] = {
				{ "Scale", std::move(jScale) },
				{ "State", stateStr }
			};
		}

		out["Physics"] = std::move(jPhys);
	}

	// === Shader Resources ===
	{
		const auto& s = m_ShaderResources;
		nlohmann::json jS = nlohmann::json::object();

		auto put_if = [&](const char* k, const std::string& v)
		{
			if (!v.empty()) jS[k] = v; // skip empties to keep JSON clean
		};

		put_if("Texture", s.GetTexture());
		put_if("Secondary Texture", s.GetSecondaryTexture());
		put_if("Normal Map", s.GetNormalMap());
		put_if("Alpha Map", s.GetAlphaMap());
		put_if("Height Map", s.GetHeightMap());
		put_if("AO Map", s.GetAOMap());
		put_if("Specular Map", s.GetSpecularMap());
		put_if("Emissive Map", s.GetEmissiveMap());
		put_if("Light Map", s.GetLightMap());
		put_if("Metalness Map", s.GetMetalnessMap());
		put_if("Roughness Map", s.GetRoughnessMap());
		put_if("Displacement Map", s.GetDisplacementMap());

		out["Shader"] = std::move(jS);
	}

	return out;
}

void IRender::AddLight(ILightSource* lightSource) const
{
	m_LightManager.AddLight(lightSource);
}

void IRender::RemoveLight(ILightSource* lightSource) const
{
	m_LightManager.RemoveLight(lightSource);
}

CubeCollider* IRender::GetCubeCollider() const
{
	return m_CubeCollider.get();
}

RigidBody* IRender::GetRigidBody()
{
	return &m_RigidBody;
}

void IRender::SetScale(float x, float y, float z)
{
	if (m_Scale.x != x || m_Scale.y != y || m_Scale.z == z) m_bDirty = true;
	m_Scale = { x, y, z };
}

void IRender::SetScale(const DirectX::XMFLOAT3& scale)
{
	if (m_Scale.x != scale.x || m_Scale.y != scale.y || m_Scale.z != scale.z)
		m_bDirty = true;
	m_Scale = scale;
}

void IRender::SetScale(const DirectX::XMVECTOR& scale)
{
	DirectX::XMFLOAT3 newScale;
	XMStoreFloat3(&newScale, scale);

	if (m_Scale.x != newScale.x || m_Scale.y != newScale.y || m_Scale.z != newScale.z)
		m_bDirty = true;
	m_Scale = newScale;
}

void IRender::SetScaleXY(float x, float y)
{
	if (m_Scale.x != x || m_Scale.y != y)
		m_bDirty = true;

	m_Scale.x = x;
	m_Scale.y = y;
}

void IRender::SetScaleXY(const DirectX::XMFLOAT2& scale)
{
	if (m_Scale.x != scale.x || m_Scale.y != scale.y)
		m_bDirty = true;

	m_Scale.x = scale.x;
	m_Scale.y = scale.y;
}

void IRender::SetScaleX(float x)
{
	if (m_Scale.x != x)
		m_bDirty = true;

	m_Scale.x = x;
}

void IRender::SetScaleY(float y)
{
	if (m_Scale.y != y)
		m_bDirty = true;

	m_Scale.y = y;
}

void IRender::SetScaleZ(float z)
{
	if (m_Scale.z != z)
		m_bDirty = true;

	m_Scale.z = z;
}

void IRender::AddScale(float x, float y, float z)
{
	if (x != 0.0f || y != 0.0f || z != 0.0f)
		m_bDirty = true;

	m_Scale.x += x;
	m_Scale.y += y;
	m_Scale.z += z;
}

void IRender::AddScale(const DirectX::XMFLOAT3& scale)
{
	if (scale.x != 0.0f || scale.y != 0.0f || scale.z != 0.0f)
		m_bDirty = true;

	m_Scale.x += scale.x;
	m_Scale.y += scale.y;
	m_Scale.z += scale.z;
}

void IRender::AddScale(const DirectX::XMVECTOR& scale)
{
	DirectX::XMFLOAT3 temp;
	DirectX::XMStoreFloat3(&temp, scale);
	m_Scale.x += temp.x;
	m_Scale.y += temp.y;
	m_Scale.z += temp.z;
	m_bDirty = true;
}

void IRender::AddScale(float x, float y)
{
	m_Scale.x += x;
	m_Scale.y += y;
	m_bDirty = true;
}

void IRender::AddScaleXY(const DirectX::XMFLOAT2& scale)
{
	m_Scale.x += scale.x;
	m_Scale.y += scale.y;
	m_bDirty = true;
}

void IRender::AddScaleX(float x)
{
	m_Scale.x += x;
	m_bDirty = true;
}

void IRender::AddScaleY(float y)
{
	m_Scale.y += y;
	m_bDirty = true;
}

void IRender::AddScaleZ(float z)
{
	m_Scale.z += z;
	m_bDirty = true;
}

DirectX::XMFLOAT3 IRender::GetScale() const
{
	return m_Scale;
}

DirectX::XMFLOAT2 IRender::GetScaleXY() const
{
	return { m_Scale.x, m_Scale.y };
}

float IRender::GetScaleX() const
{
	return m_Scale.x;
}

float IRender::GetScaleY() const
{
	return m_Scale.y;
}

float IRender::GetScaleZ() const
{
	return m_Scale.z;
}

void IRender::SetTextureMultiplier(int valueX, int valueY)
{
	m_TextureMultiplierX = valueX;
	m_TextureMultiplierY = valueY;
	ResetInitialization();
}

bool IRender::IsTransparent() const
{
	return m_bTransparent;
}

void IRender::SetTransparent(bool flag)
{
	m_bTransparent = flag;
}

DirectX::XMMATRIX IRender::GetNormalTransform() const
{
	using namespace DirectX;

	XMMATRIX scaleMat = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotMat = m_RigidBody.GetOrientation().ToRotationMatrix();

	XMMATRIX worldMat = scaleMat * rotMat;
	XMMATRIX normalMat = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMat));
	return normalMat;
}

ShaderResource* IRender::GetShaderResource()
{
	return &m_ShaderResources;
}

PIXEL_BUFFER_METADATA_GPU IRender::GetPixelCBMetaData() const
{
	PIXEL_BUFFER_METADATA_GPU data{};

	auto lightData = m_LightManager.GetLightMetaDataInfo();

	data.SpotLightCount = lightData.SpotLightCount;
	data.DirectionalLightCount = lightData.DirectionLightCount;
	data.PointLightCount = lightData.PointLightCount;

	data.DebugLine = 0;

	data.Texture = m_ShaderResources.IsTextureInitialized();
	data.MultiTexturing = m_ShaderResources.IsSecondaryTextureInitialized();
	data.LightMap = m_ShaderResources.IsLightMapInitialized();
	data.AlphaMap = m_ShaderResources.IsAlphaMapInitialized();
	data.AlphaValue = m_ShaderResources.GetAlphaValue();
	data.NormalMap = m_ShaderResources.IsNormalMapInitialized();

	data.HeightMap = m_ShaderResources.IsHeightMapInitialized();
	data.RoughnessMap = m_ShaderResources.IsRoughnessMapInitialized();
	data.MetalnessMap = m_ShaderResources.IsMetalnessMapInitialized();
	data.AOMap = m_ShaderResources.IsAOMapInitialized();
	data.SpecularMap = m_ShaderResources.IsSpecularMapInitialized();
	data.EmissiveMap = m_ShaderResources.IsEmissiveMapInitialized();
	data.DisplacementMap = m_ShaderResources.IsDisplacementMapInitialized();

	return data;
}

void IRender::PrintMatrix(const DirectX::XMMATRIX& mat)
{
	using namespace DirectX;
	XMFLOAT4X4 debug;
	XMStoreFloat4x4(&debug, mat);

	LOG_INFO("Normal Matrix:");
	for (int i = 0; i < 4; ++i)
	{
		std::string data_1 = std::to_string(debug.m[i][0]);
		std::string data_2 = std::to_string(debug.m[i][1]);
		std::string data_3 = std::to_string(debug.m[i][2]);
		std::string data_4 = std::to_string(debug.m[i][3]);
		LOG_INFO(data_1 + ", " + data_2 + ", " + data_3 + ", " + data_4 + "\n");
	}
}

std::string IRender::OpenFileDialog(const char* filter)
{
	char filename[MAX_PATH]{};

	OPENFILENAMEA ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn))
	{
		std::string fullPath = filename;

		// Look for "EntityUnknown" in the path
		size_t rootPos = fullPath.find(ROOT_PATH);
		if (rootPos != std::string::npos)
		{
			// Skip the folder name itself to keep relative structure
			size_t relativeStart = rootPos + strlen(ROOT_PATH);

			// Ensure it starts with a slash
			if (fullPath[relativeStart] == '\\' || fullPath[relativeStart] == '/')
				relativeStart++;

			std::string relativePath = fullPath.substr(relativeStart);
			return relativePath;
		}

		// fallback: absolute path
		return fullPath;
	}

	return {};
}

void IRender::EnableLight(bool flag)
{
	m_LightEnabled = flag;
}

void IRender::UpdateVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const
{
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return;

	if (m_VertexMetadataCB)
	{
		m_VertexMetadataCB->Update(deviceContext, &m_WorldMatrixGPU);
	}
}

void IRender::UpdateVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, const VERTEX_BUFFER_METADATA_GPU& gpuData) const
{
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return;

	if (m_VertexMetadataCB)
	{
		m_VertexMetadataCB->Update(deviceContext, &gpuData);
	}
}

void IRender::UpdatePixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, bool debug) const
{
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return;

	auto data = GetPixelCBMetaData();
	data.DebugLine = debug;

	if (m_PixelMetadataCB)
	{
		m_PixelMetadataCB->Update(deviceContext, &data);
	}
}

void IRender::UpdatePixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, const PIXEL_BUFFER_METADATA_GPU& gpuData) const
{
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return;

	if (m_PixelMetadataCB)
	{
		m_PixelMetadataCB->Update(deviceContext, &gpuData);
	}
}

void IRender::BindVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const
{
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return;

	if (m_VertexMetadataCB)
	{
		deviceContext->VSSetConstantBuffers(m_VertexMetadataCB_Slot, 1u, m_VertexMetadataCB->GetAddressOf());
	}
}

void IRender::BindPixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const
{
	if (m_bRenderOnDebugOnly && EDITOR_STATE::PLAY_STATE) return;

	if (m_PixelMetadataCB)
	{
		deviceContext->PSSetConstantBuffers(m_PixelMetadataCB_Slot, 1u, m_PixelMetadataCB->GetAddressOf());
	}
}

void IRender::UI_SafeCopy(char* dst, size_t dstSize, const std::string& src)
{
	if (!dst || dstSize == 0) return;
	std::memset(dst, 0, dstSize);
	const size_t n = std::min(src.size(), dstSize - 1);
	if (n) std::memcpy(dst, src.data(), n);
	dst[dstSize - 1] = '\0';
}

int IRender::UI_TopologyToIndex(D3D_PRIMITIVE_TOPOLOGY t)
{
	switch (t)
	{
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:  return 0;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return 1;
	case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:      return 2;
	case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:     return 3;
	case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST:     return 4;
	default: return 0;
	}
}

D3D_PRIMITIVE_TOPOLOGY IRender::UI_IndexToTopology(int idx)
{
	switch (idx)
	{
	default:
	case 0: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case 1: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case 2: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case 3: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case 4: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	}
}

void IRender::UI_PathFieldWithApplyAndDnD(const char* label,
	const std::string& currentValue,
	const std::function<void(const std::string&)>& applySetter,
	bool showPreview) const
{
	using CB = ImGuiContentBrowserPolicy;

	ImGui::PushID(label);
	const ImGuiID id = ImGui::GetID("##PathField");
	auto& buf = UIHelpers::g_PathBuffers[id];

	if (buf[0] == '\0' && !currentValue.empty())
		UI_SafeCopy(buf.data(), buf.size(), currentValue);

	ImGui::TextUnformatted(label);
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110.0f);
	const std::string hidden = std::string("##") + label;
	ImGui::InputText(hidden.c_str(), buf.data(), buf.size());

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(CB::kPayloadType))
		{
			CB::PayloadHeader hdr{};
			std::string pathUtf8;
			if (CB::ParsePayload(payload, hdr, pathUtf8) && (CB::Kind)hdr.kind == CB::Kind::File)
				UI_SafeCopy(buf.data(), buf.size(), pathUtf8);
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::SameLine();
	if (ImGui::Button("Apply"))
		applySetter(std::string(buf.data()));

	ImGui::SameLine();
	if (ImGui::SmallButton("X"))
		buf[0] = '\0';

	if (showPreview)
	{
		std::string p = buf.data();
		auto ext = std::filesystem::path(p).extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char c) { return (char)std::tolower(c); });

		if (!p.empty() &&
			(ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" ||
				ext == ".dds" || ext == ".hdr" || ext == ".tif" || ext == ".tiff" || ext == ".webp"))
		{
			auto tex = TextureLoader::GetTexture(p.c_str());
			if (tex.IsInitialized() && tex.ShaderResourceView)
			{
				ImGui::SameLine();
				ImGui::Image((ImTextureID)tex.ShaderResourceView, ImVec2(32, 32));
			}
		}
	}

	ImGui::Spacing();
	ImGui::PopID();
}

void IRender::UI_Section_ObjectAndRender(LevelEditorContext*)
{
	if (ImGui::CollapsingHeader("Object & Render", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// ---- Object name ----
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
		{
			// apply on button press
		}
		ImGui::SameLine();
		if (ImGui::Button("Rename"))
			SetName(std::string(nameBuffer));

		ImGui::Separator();

		// ---- Mesh & Topology ----
		{
			const char* topoLabels[] = { "Triangle List", "Triangle Strip", "Line List", "Line Strip", "Point List" };
			int topoIndex = UI_TopologyToIndex(m_PrimitiveTopology);
			if (ImGui::Combo("Primitive Topology", &topoIndex, topoLabels, IM_ARRAYSIZE(topoLabels)))
				m_PrimitiveTopology = UI_IndexToTopology(topoIndex);
		}

		ImGui::Separator();

		// ---- Material ----
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

		// ---- Texture Multiplier ----
		{
			static int xMultiplier = 1;
			static int yMultiplier = 1;

			ImGui::DragInt("Tile X", &xMultiplier, 0.1f, 1, 64);
			ImGui::DragInt("Tile Y", &yMultiplier, 0.1f, 1, 64);

			if (ImGui::Button("Apply Texture Multiplier"))
				SetTextureMultiplier(xMultiplier, yMultiplier);
		}
	}
}

void IRender::UI_Section_TransformAndPhysics(LevelEditorContext*)
{
	if (ImGui::CollapsingHeader("Transform & Physics", ImGuiTreeNodeFlags_DefaultOpen))
	{
		DirectX::XMFLOAT3 pos = m_RigidBody.GetTranslation();
		if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
			m_RigidBody.SetTranslation(pos.x, pos.y, pos.z);

		{
			Quaternion q = m_RigidBody.GetOrientation();
			float orientation[4] = { q.GetI(), q.GetJ(), q.GetK(), q.GetR() };
			if (ImGui::DragFloat4("Orientation (x, y, z, w)", orientation, 0.01f))
			{
				Quaternion updated(orientation[3], orientation[0], orientation[1], orientation[2]);
				m_RigidBody.SetOrientation(updated);
			}
		}

		DirectX::XMFLOAT3 vel; XMStoreFloat3(&vel, m_RigidBody.GetVelocity());
		if (ImGui::DragFloat3("Velocity", &vel.x, 0.01f))
			m_RigidBody.SetVelocity({ vel.x, vel.y, vel.z });

		DirectX::XMFLOAT3 acc; XMStoreFloat3(&acc, m_RigidBody.GetAcceleration());
		if (ImGui::DragFloat3("Acceleration", &acc.x, 0.01f))
			m_RigidBody.SetAcceleration({ acc.x, acc.y, acc.z });

		DirectX::XMFLOAT3 scale = GetScale();
		if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
			SetScale(scale);

		if (CubeCollider* cube = GetCubeCollider())
		{
			DirectX::XMVECTOR s = cube->GetScale();
			DirectX::XMFLOAT3 cs; XMStoreFloat3(&cs, s);
			if (ImGui::DragFloat3("Collider Scale", &cs.x, 0.01f))
				cube->SetScale(DirectX::XMLoadFloat3(&cs));

			static const char* stateLabels[] = { "Dynamic", "Static", "Trigger" };
			int stateIndex = static_cast<int>(cube->GetColliderState());
			if (ImGui::Combo("Collider State", &stateIndex, stateLabels, IM_ARRAYSIZE(stateLabels)))
				cube->SetColliderState(static_cast<ColliderState>(stateIndex));
		}
	}
}

void IRender::UI_Section_Textures(LevelEditorContext*)
{
	if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
	{
		auto& shader = m_ShaderResources;

		UI_PathFieldWithApplyAndDnD("Texture", shader.GetTexture(), [&](const std::string& p) { shader.SetTexture(p); });
		UI_PathFieldWithApplyAndDnD("Secondary Texture", shader.GetSecondaryTexture(), [&](const std::string& p) { shader.SetSecondaryTexture(p); });
		UI_PathFieldWithApplyAndDnD("Light Map", shader.GetLightMap(), [&](const std::string& p) { shader.SetLightMap(p); });
		UI_PathFieldWithApplyAndDnD("Alpha Map", shader.GetAlphaMap(), [&](const std::string& p) { shader.SetAlphaMap(p); });
		UI_PathFieldWithApplyAndDnD("Normal Map", shader.GetNormalMap(), [&](const std::string& p) { shader.SetNormalMap(p); });
		UI_PathFieldWithApplyAndDnD("Height Map", shader.GetHeightMap(), [&](const std::string& p) { shader.SetHeightMap(p); });
		UI_PathFieldWithApplyAndDnD("Roughness Map", shader.GetRoughnessMap(), [&](const std::string& p) { shader.SetRoughnessMap(p); });
		UI_PathFieldWithApplyAndDnD("Metalness Map", shader.GetMetalnessMap(), [&](const std::string& p) { shader.SetMetalnessMap(p); });
		UI_PathFieldWithApplyAndDnD("AO Map", shader.GetAOMap(), [&](const std::string& p) { shader.SetAOMap(p); });
		UI_PathFieldWithApplyAndDnD("Specular Map", shader.GetSpecularMap(), [&](const std::string& p) { shader.SetSpecularMap(p); });
		UI_PathFieldWithApplyAndDnD("Emissive Map", shader.GetEmissiveMap(), [&](const std::string& p) { shader.SetEmissiveMap(p); });
		UI_PathFieldWithApplyAndDnD("Displacement Map", shader.GetDisplacementMap(), [&](const std::string& p) { shader.SetDisplacementMap(p); });
	}
}
