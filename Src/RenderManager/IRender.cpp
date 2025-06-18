#include "IRender.h"

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
	data.LightMap = m_ShaderResources.IsLightMapInitialized();
	data.MultiTexturing = m_ShaderResources.IsSecondaryTextureInitialized();
	data.AlphaMap = m_ShaderResources.IsAlphaMapInitialized();
	data.AlphaValue = m_ShaderResources.GetAlphaValue();
	data.NormalMap = m_ShaderResources.IsNormalMapInitialized();
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

void IRender::UpdatePixelMetaDataConstantBuffer(ID3D11DeviceContext* deviceContext, bool debug) const
{
	auto data = GetPixelCBMetaData();
	data.DebugLine = debug;

	if (m_PixelMetadataCB)
	{
		m_PixelMetadataCB->Update(deviceContext, &data);
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
