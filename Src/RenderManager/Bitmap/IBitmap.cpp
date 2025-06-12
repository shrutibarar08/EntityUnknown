#include "IBitmap.h"


void IBitmap::SetScreenSize(int x, int y)
{
	m_ScreenHeight = y;
	m_ScreenWidth = x;
}

void IBitmap::SetScale(float x, float y, float z)
{
	m_Scale.x = x;
	m_Scale.y = y;
	m_Scale.z = z;
}

void IBitmap::SetRenderPosition(float x, float y)
{
	m_PosX = x;
	m_PosY = y;
}

bool IBitmap::IsInitialized() const
{
	return m_Initialized;
}

bool IBitmap::Build(ID3D11Device* device)
{
	if (m_Initialized) return true;
	LOG_INFO("Starting to build Bitmap");

	m_SharedBitMapBuffer = std::make_unique<BitMapBuffer>(6, 6);
	m_BitMapBuffer = std::make_unique<BitmapInstance<BitMapBuffer>>(m_SharedBitMapBuffer);
	m_BitMapBuffer->Init(device);

	//~ Build Shaders
	m_ShaderResources = std::make_unique<ShaderResource>();
	m_ShaderResources->AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources->AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	//m_ShaderResources->AddTexture("Texture/health.tga");
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

	//~ Vertex Constant Buffer
	m_VertexConstantBuffer = std::make_unique<ConstantBuffer<WORLD_2D_TRANSFORM>>(device);

	if (!m_ShaderResources->Build(device))
	{
		return false;
	}

	std::string message = "Initialized Cube Model: Index Count: " + std::to_string(m_BitMapBuffer->GetIndexCount());
	LOG_INFO(message);

	LOG_INFO("Build Complete");
	m_Initialized = true;
	return true;
}

bool IBitmap::Render(ID3D11DeviceContext* deviceContext)
{
	if (!IsInitialized()) return false;

	UpdateVertexBuffer(deviceContext);
	m_ShaderResources->Render(deviceContext);
	m_VertexConstantBuffer->Update(deviceContext, &m_WorldTransform);
	deviceContext->VSSetConstantBuffers(
		0u,
		1u,
		m_VertexConstantBuffer->GetAddressOf()
	);

	m_BitMapBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return true;
}

void IBitmap::UpdateTransformation(const CAMERA_2D_MATRIX_DESC& cameraInfo)
{
	auto resource = m_ShaderResources->GetTextureResource();
	auto scale = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	auto translation = DirectX::XMMatrixTranslation(m_PosX, m_PosY, 0.0f);

	m_WorldTransform.WorldMatrix = DirectX::XMMatrixTranspose(scale * translation);
	m_WorldTransform.ViewMatrix = cameraInfo.ViewMatrix;
	m_WorldTransform.ProjectionMatrix = cameraInfo.ProjectionMatrix;
}

void IBitmap::SetTexture(const std::string& texture)
{
	m_TextureToAdd = texture;
}

void IBitmap::UpdateVertexBuffer(ID3D11DeviceContext* deviceContext)
{
	if (!IsInitialized()) return;

	if ((m_PrevX == m_PosX) && (m_PrevY == m_PosY)) return;
	m_PrevX = m_PosX;
	m_PrevY = m_PosY;

	TEXTURE_RESOURCE resource = m_ShaderResources->GetTextureResource();
	if (!resource.IsInitialized()) return;

	float left = static_cast<float>(-m_ScreenWidth) / 2.0f + static_cast<float>(m_PosX);
	float right = left + static_cast<float>(resource.Width);
	float top = static_cast<float>(m_ScreenHeight) / 2.0f - static_cast<float>(m_PosY);
	float bottom = top - static_cast<float>(resource.Height);

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
