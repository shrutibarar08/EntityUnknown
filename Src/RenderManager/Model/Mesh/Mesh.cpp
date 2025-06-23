#include "Mesh.h"

#include "ExceptionManager/IException.h"
#include "RenderManager/Model/ModelLoader/ObjLoader/ObjLoader.h"

void Mesh::SetMeshPath(const std::string& path)
{
	m_MeshPath = path;
}

bool Mesh::IsInitialized() const
{
	return m_Initialized;
}

bool Mesh::BuildChild(ID3D11Device* device)
{
	if (m_MeshPath.empty()) return false;
	if (m_Initialized) return true;

	
	auto sharedMesh = ObjLoader::Load(m_MeshPath);

	if (sharedMesh == nullptr) THROW("Failed to Load Mesh");

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
