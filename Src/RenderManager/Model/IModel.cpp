#include "IModel.h"
#include "RenderManager/Components/ShaderResource/LightBuffer/LightBuffer.h"


bool IModel::Build(ID3D11Device* device)
{
	m_LightBufferManager.RegisterBuffer<DirectionalBufferConfig>(0);
	m_LightBufferManager.BuildAll(device);

	if (!m_bModelCommonDataInitialized)
	{
		LOG_WARNING("3D Common Data Constant Buffer Initialized");
		m_bModelCommonDataInitialized = true;
		m_PixelMetadataCB = std::make_unique<ConstantBuffer<PIXEL_BUFFER_METADATA_GPU>>(device);
		m_3DModelConstantBuffer = std::make_unique<ConstantBuffer<TRANSFORM_3D_GPU>>(device);
		m_WorldMatrixConstantBuffer = std::make_unique<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>>(device);
	}

	return BuildChild(device);
}

bool IModel::Render(ID3D11DeviceContext* deviceContext)
{
	if (!m_bModelCommonDataInitialized) return false;

	//~ Updates World Matrix Constant Buffer
	m_WorldMatrixConstantBuffer->Update(deviceContext, &m_WorldMatrix);
	deviceContext->VSSetConstantBuffers(0u, 1u, m_WorldMatrixConstantBuffer->GetAddressOf());

	//~ Updates 3D Model related information on constant buffer
	m_3DModelConstantBuffer->Update(deviceContext, &m_3DTransformData);
	deviceContext->VSSetConstantBuffers(1u, 1u, m_3DModelConstantBuffer->GetAddressOf());

	//~ Updates Light Meta data
	PIXEL_BUFFER_METADATA_GPU meta{};
	meta.DirectionalLightCount = 10;
	meta.DebugLine = 0;

	m_PixelMetadataCB->Update(deviceContext, &meta);
	deviceContext->PSSetConstantBuffers(0u, 1u, m_PixelMetadataCB->GetAddressOf());

	//~ Attach Light Sources data into the struct array (GPU Side).
	DirectX::XMFLOAT3 position = m_RigidBody.GetTranslation();
	m_LightBufferManager.RenderAll(position, deviceContext);

	return RenderChild(deviceContext);
}

void IModel::AddLight(ILightAnyType* lightSource)
{
	m_LightBufferManager.AddLightToAll(lightSource);
}

void IModel::RemoveLight(ILightAnyType* lightSource)
{
	m_LightBufferManager.RemoveLightFromAll(lightSource);
}

void IModel::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
{
	// Get transform components
	DirectX::XMFLOAT3 scale = GetScale();
	DirectX::XMFLOAT3 translation = m_RigidBody.GetTranslation();
	DirectX::XMFLOAT3 rotation = m_RigidBody.GetRotation(); // pitch (X), yaw (Y), roll (Z)

	// Build transformation matrix
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);

	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(S * R * T);

	// Store in world constant buffer
	m_WorldMatrix.WorldMatrix = worldMatrix;
	m_WorldMatrix.ViewMatrix = cameraInfo.ViewMatrix;
	m_WorldMatrix.ProjectionMatrix = cameraInfo.ProjectionMatrix;
	m_WorldMatrix.CameraPosition = cameraInfo.CameraPosition;
	m_WorldMatrix.Padding = 0.0f;

	m_3DTransformData.NormalMatrix = GetNormalTransform();
}
