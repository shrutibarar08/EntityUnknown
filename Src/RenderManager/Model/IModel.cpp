#include "IModel.h"

#include <iostream>

#include "Utils/Logger/Logger.h"


bool IModel::Build(ID3D11Device* device)
{
	m_LightManager.Build(device);
	m_3DModelConstantBuffer = std::make_unique<ConstantBuffer<NORMAL_3D_GPU>>(device);

	if (!m_bModelCommonDataInitialized)
	{
		LOG_WARNING("3D Common Data Constant Buffer Initialized");
		m_bModelCommonDataInitialized = true;
		m_PixelMetadataCB = std::make_unique<ConstantBuffer<PIXEL_BUFFER_METADATA_GPU>>(device);
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
	LIGHT_META_DATA data = m_LightManager.GetLightMetaDataInfo();
	m_PixelMetaData.SpotLightCount = data.SpotLightCount;
	m_PixelMetaData.PointLightCount = data.PointLightCount;
	m_PixelMetaData.DirectionalLightCount = data.DirectionLightCount;
	m_PixelMetaData.DebugLine = 0;
	m_PixelMetaData.MultiTexturing = IsMultiTextureEnable();

	m_PixelMetadataCB->Update(deviceContext, &m_PixelMetaData);
	deviceContext->PSSetConstantBuffers(0u, 1u, m_PixelMetadataCB->GetAddressOf());

	//~ Attach Light Sources data into the struct array (GPU Side).
	m_LightManager.Update(deviceContext, m_RigidBody.GetPosition());
	m_LightManager.Bind(deviceContext);

	return RenderChild(deviceContext);
}

void IModel::AddLight(ILightSource* lightSource) const
{
	m_LightManager.AddLight(lightSource);
}

void IModel::RemoveLight(ILightSource* lightSource) const
{
	m_LightManager.RemoveLight(lightSource);
}

void IModel::SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo)
{
	// Get transform components
	DirectX::XMFLOAT3 scale = GetScale();
	DirectX::XMFLOAT3 translation = m_RigidBody.GetTranslation();

	// Build transformation matrix
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = m_RigidBody.GetOrientation().ToRotationMatrix();
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

void IModel::PrintMatrix(const DirectX::XMMATRIX& mat)
{
	using namespace DirectX;
	XMFLOAT4X4 debug;
	XMStoreFloat4x4(&debug, mat);

	std::cout << "Normal Matrix:\n";
	for (int i = 0; i < 4; ++i)
	{
		std::cout << debug.m[i][0] << ", "
			<< debug.m[i][1] << ", "
			<< debug.m[i][2] << ", "
			<< debug.m[i][3] << "\n";
	}
}
