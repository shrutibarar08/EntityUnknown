#include "IRender.h"

#include <format>
#include <__msvc_filebuf.hpp>

#include "Imgui/imgui.h"

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

	BuildShaders(device, deviceContext);
	m_LightManager.Build(device);

	return true;
}

bool IRender::Render(ID3D11DeviceContext* deviceContext)
{
	if (!m_bCommonDataInitialized) return false;

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

void IRender::RenderControlUI()
{
	// --- Name Control with Rename button ---
	{
		static char nameBuffer[128]{};

		static uintptr_t lastObjectID = 0;
		uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
		if (lastObjectID != currentID)
		{
			lastObjectID = currentID;
			std::string currentName = GetName();
			strncpy_s(nameBuffer, currentName.c_str(), sizeof(nameBuffer));
		}

		ImGui::InputText("Object Name", nameBuffer, sizeof(nameBuffer));
		ImGui::SameLine();
		if (ImGui::Button("Rename"))
		{
			SetName(std::string(nameBuffer));
		}
	}
	ImGui::Separator();

	// --- Shader Resource Controls ---
	ImGui::Text("Shader Textures");

	auto& shader = m_ShaderResources;

	auto textureControl = [this](const char* label, const std::string& currentPath, auto setter)
		{
			ImGui::PushID(label);
			char buffer[256]{};
			strncpy_s(buffer, currentPath.c_str(), sizeof(buffer));
			ImGui::InputText(label, buffer, sizeof(buffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse"))
			{
				std::string path = OpenFileDialog("Image Files\0*.png;*.jpg;*.dds;*.tga\0All Files\0*.*\0");
				if (!path.empty())
					setter(path);
			}
			ImGui::PopID();
		};

	textureControl("Texture", shader.GetTexture(), [&](const std::string& p) { shader.SetTexture(p); });
	textureControl("Secondary Texture", shader.GetSecondaryTexture(), [&](const std::string& p) { shader.SetSecondaryTexture(p); });
	textureControl("Light Map", shader.GetLightMap(), [&](const std::string& p) { shader.SetLightMap(p); });
	textureControl("Alpha Map", shader.GetAlphaMap(), [&](const std::string& p) { shader.SetAlphaMap(p); });
	textureControl("Normal Map", shader.GetNormalMap(), [&](const std::string& p) { shader.SetNormalMap(p); });
	textureControl("Height Map", shader.GetHeightMap(), [&](const std::string& p) { shader.SetHeightMap(p); });
	textureControl("Roughness Map", shader.GetRoughnessMap(), [&](const std::string& p) { shader.SetRoughnessMap(p); });
	textureControl("Metalness Map", shader.GetMetalnessMap(), [&](const std::string& p) { shader.SetMetalnessMap(p); });
	textureControl("AO Map", shader.GetAOMap(), [&](const std::string& p) { shader.SetAOMap(p); });
	textureControl("Specular Map", shader.GetSpecularMap(), [&](const std::string& p) { shader.SetSpecularMap(p); });
	textureControl("Emissive Map", shader.GetEmissiveMap(), [&](const std::string& p) { shader.SetEmissiveMap(p); });
	textureControl("Displacement Map", shader.GetDisplacementMap(), [&](const std::string& p) { shader.SetDisplacementMap(p); });

	// --- Transform Controls ---
	ImGui::Separator();
	ImGui::Text("Transform & Physics");

	DirectX::XMFLOAT3 pos = m_RigidBody.GetTranslation();
	if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
		m_RigidBody.SetTranslation(pos.x, pos.y, pos.z);

	// === Orientation ===
	Quaternion q = m_RigidBody.GetOrientation();
	float orientation[4] = { q.GetI(), q.GetI(), q.GetK(), q.GetR() };

	if (ImGui::DragFloat4("Orientation (x, y, z, w)", orientation, 0.01f))
	{
		Quaternion updated(orientation[3], orientation[0], orientation[1], orientation[2]);
		m_RigidBody.SetOrientation(updated);
	}

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

	DirectX::XMVECTOR colliderScale = GetCubeCollider()->GetScale();
	DirectX::XMFLOAT3 colScale;
	XMStoreFloat3(&colScale, colliderScale);
	if (ImGui::DragFloat3("Collider Scale", &colScale.x, 0.01f))
		GetCubeCollider()->SetScale(DirectX::XMLoadFloat3(&colScale));
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
	if (m_VertexMetadataCB)
	{
		m_VertexMetadataCB->Update(deviceContext, &m_WorldMatrixGPU);
	}
}

void IRender::UpdateVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, const VERTEX_BUFFER_METADATA_GPU& gpuData) const
{
	if (m_VertexMetadataCB)
	{
		m_VertexMetadataCB->Update(deviceContext, &gpuData);
	}
}

void IRender::UpdatePixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, bool debug) const
{
	auto data = GetPixelCBMetaData();
	data.DebugLine = debug;

	if (m_PixelMetadataCB)
	{
		m_PixelMetadataCB->Update(deviceContext, &data);
	}
}

void IRender::UpdatePixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, const PIXEL_BUFFER_METADATA_GPU& gpuData) const
{
	if (m_PixelMetadataCB)
	{
		m_PixelMetadataCB->Update(deviceContext, &gpuData);
	}
}

void IRender::BindVertexMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const
{
	if (m_VertexMetadataCB)
	{
		deviceContext->VSSetConstantBuffers(m_VertexMetadataCB_Slot, 1u, m_VertexMetadataCB->GetAddressOf());
	}
}

void IRender::BindPixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext) const
{
	if (m_PixelMetadataCB)
	{
		deviceContext->PSSetConstantBuffers(m_PixelMetadataCB_Slot, 1u, m_PixelMetadataCB->GetAddressOf());
	}
}
