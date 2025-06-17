#include "IModel.h"

bool IModel::Build(ID3D11Device* device)
{
	IRender::Build(device);
	return BuildChild(device);
}

bool IModel::Render(ID3D11DeviceContext* deviceContext)
{
	if (!IRender::Render(deviceContext))
	{
		return false;
	}
	return RenderChild(deviceContext);
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
	m_WorldMatrixGPU.WorldMatrix = worldMatrix;
	m_WorldMatrixGPU.ViewMatrix = cameraInfo.ViewMatrix;
	m_WorldMatrixGPU.ProjectionMatrix = cameraInfo.ProjectionMatrix;
	m_WorldMatrixGPU.NormalMatrix = GetNormalTransform();
	m_WorldMatrixGPU.CameraPosition = cameraInfo.CameraPosition;
	m_WorldMatrixGPU.Padding = 0.0f;
}
