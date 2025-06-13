#include "IBitmap.h"


bool IBitmap::IsInitialized() const
{
	return m_LocalInitialized;
}

bool IBitmap::Build(ID3D11Device* device)
{
	if (!m_bStaticInitialized)
	{
		LOG_WARNING("Render World Matrix Constant Buffer Initialized");
		m_bStaticInitialized = true;
		m_BitmapWorldMatrixCB = std::make_unique<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>>(device);

		m_SharedBitMapBuffer = std::make_unique<BitMapBuffer>(6, 6);
		m_BitMapBuffer = std::make_unique<BitmapInstance<BitMapBuffer>>(m_SharedBitMapBuffer);
		m_BitMapBuffer->Init(device);
	}

	if (m_LocalInitialized) return true;
	m_LocalInitialized = true;

	//~ Build Shaders
	m_ShaderResources = std::make_unique<ShaderResource>();
	m_ShaderResources->AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources->AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	m_ShaderResources->AddTexture(m_TextureToAdd);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = L"Shader/Bitmap/BitmapVS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources->SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = L"Shader/Bitmap/BitmapPS.hlsl";
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "ps_5_0";
	m_ShaderResources->SetPixelShaderPath(vertexDesc);

	if (!m_ShaderResources->Build(device))
	{
		return false;
	}

	return true;
}

bool IBitmap::Render(ID3D11DeviceContext* deviceContext)
{
	if (!IsInitialized()) return false;

	UpdateVertexBuffer(deviceContext);
	m_ShaderResources->Render(deviceContext);

	m_BitmapWorldMatrixCB->Update(deviceContext, &m_WorldMatrix);
	deviceContext->VSSetConstantBuffers(
		0u,
		1u,
		m_BitmapWorldMatrixCB->GetAddressOf());

	m_BitMapBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return false;
}

void IBitmap::SetTexture(const std::string& texture)
{
	m_TextureToAdd = texture;
}

void IBitmap::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
{
	// Get transform components
	DirectX::XMFLOAT2 scale = GetScaleXY();
	DirectX::XMFLOAT2 translation = GetTranslationXY();

	// Build transformation matrix
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, 1.f);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationZ(GetYaw());
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(translation.x, translation.y, 0.0f);

	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(S * R * T);

	// Store in world constant buffer
	m_WorldMatrix.WorldMatrix = worldMatrix;
	m_WorldMatrix.ViewMatrix = cameraInfo.ViewMatrix;
	m_WorldMatrix.ProjectionMatrix = cameraInfo.ProjectionMatrix;
	m_WorldMatrix.CameraPosition = cameraInfo.CameraPosition;
	m_WorldMatrix.Padding = 0.0f;
}

void IBitmap::UpdateVertexBuffer(ID3D11DeviceContext* deviceContext) const
{
	if (m_bDynamicBufferInitialized) return;
	m_bDynamicBufferInitialized = true;

	std::vector<Vertex2D> vertices(6);
	// Unit quad at origin [-0.5, 0.5]
	vertices[0] = { {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f} }; // TL
	vertices[1] = { { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f} }; // BR
	vertices[2] = { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} }; // BL
	vertices[3] = { {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f} };
	vertices[4] = { { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f} };
	vertices[5] = { { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f} };

	m_BitMapBuffer->Update(deviceContext, vertices);
}
