#include "ILightSource.h"

#include "Utils/Logger/Logger.h"


void ILightSource::ComputeViewMatrix(const DirectX::XMVECTOR& targetPosition)
{
	using namespace DirectX;

	XMFLOAT3 lightPosFloat3 = GetLightPosition();
	XMVECTOR lightPosition = XMLoadFloat3(&lightPosFloat3);

	XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//m_ViewMatrix = XMMatrixLookAtLH(lightPosition, targetPosition, upDirection);
}

DirectX::XMINT2 ILightSource::GetShadowResolution() const
{
	return { static_cast<int>(m_ShadowWidth), static_cast<int>(m_ShadowHeight) };
}

void ILightSource::PrintLightMatrix(const DirectX::XMMATRIX& mat)
{
	using namespace DirectX;
	XMFLOAT4X4 debug;
	XMStoreFloat4x4(&debug, mat);

	for (int i = 0; i < 4; ++i)
	{
		std::string data_1 = std::to_string(debug.m[i][0]);
		std::string data_2 = std::to_string(debug.m[i][1]);
		std::string data_3 = std::to_string(debug.m[i][2]);
		std::string data_4 = std::to_string(debug.m[i][3]);
		LOG_INFO(data_1 + ", " + data_2 + ", " + data_3 + ", " + data_4 + "\n");
	}
}
