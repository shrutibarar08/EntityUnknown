#include "Mesh.h"

#include "ExceptionManager/IException.h"
#include "Imgui/imgui.h"
#include "RenderManager/Model/ModelLoader/ObjLoader/ObjLoader.h"

void Mesh::SetMeshPath(const std::string& path)
{
	m_MeshPath = path;
}

bool Mesh::IsInitialized() const
{
	return m_Initialized;
}

void Mesh::RenderControlUI()
{
	// === Name Control ===
	static char nameBuffer[128]{};
	static uintptr_t lastObjectID = 0;
	uintptr_t currentID = reinterpret_cast<uintptr_t>(this);
	if (lastObjectID != currentID)
	{
		lastObjectID = currentID;
		strncpy_s(nameBuffer, GetName().c_str(), sizeof(nameBuffer));
	}
	ImGui::InputText("Object Name", nameBuffer, sizeof(nameBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Rename"))
		SetName(std::string(nameBuffer));

	ImGui::Separator();
	ImGui::Text("Shader Textures & Mesh Path");

	// === Editable Buffers for paths ===
	static char meshBuffer[256]{};
	static char textureBuffers[12][256]{};

	// Initialize path buffers on object change
	static uintptr_t lastPathObjectID = 0;
	if (lastPathObjectID != currentID)
	{
		lastPathObjectID = currentID;
		strncpy_s(meshBuffer, m_MeshPath.c_str(), sizeof(meshBuffer));

		auto& shader = m_ShaderResources;
		strncpy_s(textureBuffers[0], shader.GetTexture().c_str(), sizeof(textureBuffers[0]));
		strncpy_s(textureBuffers[1], shader.GetSecondaryTexture().c_str(), sizeof(textureBuffers[1]));
		strncpy_s(textureBuffers[2], shader.GetLightMap().c_str(), sizeof(textureBuffers[2]));
		strncpy_s(textureBuffers[3], shader.GetAlphaMap().c_str(), sizeof(textureBuffers[3]));
		strncpy_s(textureBuffers[4], shader.GetNormalMap().c_str(), sizeof(textureBuffers[4]));
		strncpy_s(textureBuffers[5], shader.GetHeightMap().c_str(), sizeof(textureBuffers[5]));
		strncpy_s(textureBuffers[6], shader.GetRoughnessMap().c_str(), sizeof(textureBuffers[6]));
		strncpy_s(textureBuffers[7], shader.GetMetalnessMap().c_str(), sizeof(textureBuffers[7]));
		strncpy_s(textureBuffers[8], shader.GetAOMap().c_str(), sizeof(textureBuffers[8]));
		strncpy_s(textureBuffers[9], shader.GetSpecularMap().c_str(), sizeof(textureBuffers[9]));
		strncpy_s(textureBuffers[10], shader.GetEmissiveMap().c_str(), sizeof(textureBuffers[10]));
		strncpy_s(textureBuffers[11], shader.GetDisplacementMap().c_str(), sizeof(textureBuffers[11]));
	}

	// === Textures & Mesh Input Controls ===
	auto browseButton = [](char* buffer, const char* filter)
	{
		std::string path = OpenFileDialog(filter);
		if (!path.empty())
		{
			strncpy_s(buffer, 256, path.c_str(), 255);
		}
		};

	ImGui::InputText("Mesh Path", meshBuffer, sizeof(meshBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Browse Mesh"))
		browseButton(meshBuffer, "OBJ Files\0*.obj\0All Files\0*.*\0");

	const char* labels[] = {
		"Texture", "Secondary Texture", "Light Map", "Alpha Map", "Normal Map",
		"Height Map", "Roughness Map", "Metalness Map", "AO Map", "Specular Map",
		"Emissive Map", "Displacement Map"
	};

	for (int i = 0; i < 12; ++i)
	{
		ImGui::InputText(labels[i], textureBuffers[i], sizeof(textureBuffers[i]));
		ImGui::SameLine();
		std::string buttonLabel = "Browse##" + std::to_string(i);
		if (ImGui::Button(buttonLabel.c_str()))
			browseButton(textureBuffers[i], "Image Files\0*.png;*.jpg;*.dds;*.tga\0All Files\0*.*\0");
	}

	// === Apply Button ===
	if (ImGui::Button("Apply Resources"))
	{
		LOG_INFO("Setting Mesh Path as:" + std::string(meshBuffer));
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
	}

	ImGui::Separator();
	ImGui::Text("Transform & Physics");

	// === Transform Controls ===
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
		m_RigidBody.SetVelocity({vel.x, vel.y, vel.z});

	DirectX::XMFLOAT3 acc;
	XMStoreFloat3(&acc, m_RigidBody.GetAcceleration());
	if (ImGui::DragFloat3("Acceleration", &acc.x, 0.01f))
		m_RigidBody.SetAcceleration({ acc.x, acc.y, acc.z });

	DirectX::XMFLOAT3 scale = GetScale();
	if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
		SetScale(scale);

	DirectX::XMFLOAT3 colScale;
	XMStoreFloat3(&colScale, GetCubeCollider()->GetScale());
	if (ImGui::DragFloat3("Collider Scale", &colScale.x, 0.01f))
		GetCubeCollider()->SetScale(XMLoadFloat3(&colScale));
}

bool Mesh::BuildChild(ID3D11Device* device)
{
	if (m_MeshPath.empty()) return false;
	if (m_Initialized) return true;

	auto sharedMesh = ObjLoader::Load(m_MeshPath);

	if (sharedMesh == nullptr)
	{
		std::string message = "Failed to Load Mesh: " + m_MeshPath;
		LOG_ERROR(message.c_str());
		return false;
	}

	m_MeshBuffer = std::make_unique<StaticVBInstance<MeshBuffer>>(sharedMesh);
	m_MeshBuffer->Init(device);

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
		//UpdatePixelMetaDataConstantBuffer(deviceContext, true);
		//BindPixelMetaDataConstantBuffer(deviceContext);

		//m_WorldMatrixGPU.WorldMatrix = DirectX::XMMatrixTranspose(collider->GetTransformationMatrix());
		//UpdateVertexMetaDataConstantBuffer(deviceContext);
		//BindVertexMetaDataConstantBuffer(deviceContext);

		//m_MeshBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
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
	vertexDesc.FilePath = L"Shader/Shape/CubeShaderVS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources.SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = L"Shader/Shape/CubeShaderPS.hlsl";
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
