#include "IBitmap.h"

#include "Utils/Logger/Logger.h"


bool IBitmap::IsInitialized() const
{
	return m_LocalInitialized;
}

void IBitmap::SetVertexShader(const std::wstring& vertexShaderPath)
{
	m_VertexShaderPath = vertexShaderPath;
}

void IBitmap::SetPixelShader(const std::wstring& pixelShaderPath)
{
	m_PixelShaderPath = pixelShaderPath;
}

bool IBitmap::Build(ID3D11Device* device)
{
	m_LightBufferManager.RegisterBuffer<DirectionalBufferConfig>(0);
	m_LightBufferManager.BuildAll(device);

	if (!m_bStaticInitialized)
	{
		LOG_WARNING("Render World Matrix Constant Buffer Initialized");
		m_bStaticInitialized = true;
		m_LightMetaCB = std::make_unique<ConstantBuffer<PIXEL_LIGHT_META_GPU>>(device);
		m_BitmapWorldMatrixCB = std::make_unique<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>>(device);
	}

	if (m_LocalInitialized) return true;
	m_LocalInitialized = true;

	//~ Shared Data
	m_SharedBitMapBuffer = std::make_unique<BitMapBuffer>(6, 6);
	m_BitMapBuffer = std::make_unique<BitmapInstance<BitMapBuffer>>(m_SharedBitMapBuffer);
	m_BitMapBuffer->Init(device);

	//~ Build Shaders
	m_ShaderResources = std::make_unique<ShaderResource>();
	m_ShaderResources->AddElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	m_ShaderResources->AddElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
	m_ShaderResources->SetTexture(m_TextureToAdd);

	BLOB_BUILDER_DESC vertexDesc{};
	vertexDesc.FilePath = m_VertexShaderPath;
	vertexDesc.EntryPoint = "main";
	vertexDesc.Target = "vs_5_0";
	m_ShaderResources->SetVertexShaderPath(vertexDesc);

	BLOB_BUILDER_DESC PixelDesc{};
	vertexDesc.FilePath = m_PixelShaderPath;
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

	//~ Updates Light Meta data
	PIXEL_LIGHT_META_GPU meta{};
	meta.DirectionalLightCount = 10;
	m_LightMetaCB->Update(deviceContext, &meta);
	deviceContext->PSSetConstantBuffers(0u, 1u, m_LightMetaCB->GetAddressOf());

	//~ Attach Light Sources data into the struct array (GPU Side).
	DirectX::XMFLOAT3 position = GetTranslation();
	m_LightBufferManager.RenderAll(position, deviceContext);

	m_BitMapBuffer->Render(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	return false;
}

void IBitmap::SetTexture(const std::string& texture)
{
	m_TextureToAdd = texture;
}

void IBitmap::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
{
	// Optional scale/rotation in clip-space
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, 1.f);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationZ(GetYaw());

	// Translation not needed if vertices are in NDC
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(S * R);

	m_WorldMatrix.WorldMatrix = worldMatrix;
	m_WorldMatrix.ViewMatrix = DirectX::XMMatrixIdentity();
	m_WorldMatrix.ProjectionMatrix = DirectX::XMMatrixIdentity();
	m_WorldMatrix.CameraPosition = { 0.f, 0.f, 0.f }; // unused
	m_WorldMatrix.Padding = 0.f;
}

void IBitmap::AddLight(ILightAnyType* lightSource)
{
	m_LightBufferManager.AddLightToAll(lightSource);
}

void IBitmap::RemoveLight(ILightAnyType* lightSource)
{
	m_LightBufferManager.RemoveLightFromAll(lightSource);
}

void IBitmap::UpdateVertexBuffer(ID3D11DeviceContext* deviceContext)
{
	if (m_ScreenWidth == m_LastWidth && m_ScreenHeight == m_LastHeight) return;

	m_LastWidth = m_ScreenWidth;
	m_LastHeight = m_ScreenHeight;

	LOG_INFO("Updated Vertex Buffer with: " + std::to_string(m_LastWidth) + ", " + std::to_string(m_LastHeight));

	TEXTURE_RESOURCE resource = m_ShaderResources->GetTextureResource();
	if (!resource.IsInitialized()) return;

	float bitmapWidth = resource.Width * m_Scale.x;
	float bitmapHeight = resource.Height * m_Scale.y;

	// Convert pixel position to NDC
	float centerX_NDC = (m_Translation.x / m_ScreenWidth) * 2.0f - 1.0f;
	float centerY_NDC = 1.0f - (m_Translation.y / m_ScreenHeight) * 2.0f;

	// Convert size from pixels to NDC scale
	float halfWidth_NDC = (bitmapWidth / m_ScreenWidth);
	float halfHeight_NDC = (bitmapHeight / m_ScreenHeight);

	// Vertex positions in NDC
	float left = centerX_NDC - halfWidth_NDC;
	float right = centerX_NDC + halfWidth_NDC;
	float top = centerY_NDC + halfHeight_NDC;
	float bottom = centerY_NDC - halfHeight_NDC;

	std::vector<Vertex2D> vertices(6);
	vertices[0] = { {left,  top,    0.0f}, {0.0f, 0.0f} }; // TL
	vertices[1] = { {right, bottom, 0.0f}, {1.0f, 1.0f} }; // BR
	vertices[2] = { {left,  bottom, 0.0f}, {0.0f, 1.0f} }; // BL
	vertices[3] = { {left,  top,    0.0f}, {0.0f, 0.0f} };
	vertices[4] = { {right, top,    0.0f}, {1.0f, 0.0f} };
	vertices[5] = { {right, bottom, 0.0f}, {1.0f, 1.0f} };

	m_BitMapBuffer->Update(deviceContext, vertices);
}
