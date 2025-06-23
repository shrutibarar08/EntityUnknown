#include "DirectionalLight.h"
#include "Utils/Logger/Logger.h"


void DirectionalLight::SetAmbient(float red, float green, float blue, float alpha)
{
	m_AmbientColor = DirectX::XMFLOAT4(red, green, blue, alpha);
}

DirectX::XMFLOAT4 DirectionalLight::GetAmbientColor() const
{
	return m_AmbientColor;
}

void DirectionalLight::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_DiffuseColor = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void DirectionalLight::SetDirection(float x, float y, float z)
{
	m_Direction = DirectX::XMFLOAT3(x, y, z);
}

void DirectionalLight::SetSpecularColor(float red, float green, float blue, float alpha)
{
	m_SpecularColor = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void DirectionalLight::SetSpecularPower(float power)
{
	m_SpecularPower = power;
}

float DirectionalLight::GetSpecularPower() const
{
	return m_SpecularPower;
}

DIRECTIONAL_LIGHT_GPU_DATA DirectionalLight::GetLightData() const
{
	DIRECTIONAL_LIGHT_GPU_DATA data{};
	data.SpecularColor = m_SpecularColor;
	data.AmbientColor = m_AmbientColor;
	data.DiffuseColor = m_DiffuseColor;
	data.Direction = m_Direction;
	data.SpecularPower = m_SpecularPower;
	data.ViewProjectLightMatrix = GetLightViewProjMatrix();
	return data;
}

DirectX::XMFLOAT3 DirectionalLight::GetLightPosition() const
{
	//~ don't need to respond with distance.
	return { -10, -10, -10 };
}

void DirectionalLight::UpdateProjectionMatrix(const Frustum& sceneFrustum)
{
	using namespace DirectX;

	XMFLOAT3 center = sceneFrustum.GetCenter();
	XMFLOAT3 extents = sceneFrustum.GetExtents(); // Half-size in each axis

	// Transform frustum corners to light view space
	XMMATRIX view = m_ViewMatrix; // Already set
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	// Define 8 corners of the scene frustum in world space
	std::array<XMFLOAT3, 8> corners = sceneFrustum.GetCorners();
	std::array<XMVECTOR, 8> lightSpaceCorners;

	for (int i = 0; i < 8; ++i)
	{
		XMVECTOR worldPos = XMLoadFloat3(&corners[i]);
		lightSpaceCorners[i] = XMVector3Transform(worldPos, view);
	}

	// Find min/max in light view space
	XMVECTOR min = lightSpaceCorners[0];
	XMVECTOR max = lightSpaceCorners[0];

	for (int i = 1; i < 8; ++i)
	{
		min = XMVectorMin(min, lightSpaceCorners[i]);
		max = XMVectorMax(max, lightSpaceCorners[i]);
	}

	XMFLOAT3 min3, max3;
	XMStoreFloat3(&min3, min);
	XMStoreFloat3(&max3, max);

	// Now compute orthographic matrix that bounds the scene in light-space
	m_ProjMatrix = XMMatrixOrthographicOffCenterLH(
		min3.x, max3.x,
		min3.y, max3.y,
		min3.z, max3.z
	);
}

DirectX::XMFLOAT4 DirectionalLight::GetSpecularColor() const
{
	return m_SpecularColor;
}

DirectX::XMFLOAT4 DirectionalLight::GetDiffuseColor() const
{
	return m_DiffuseColor;
}

DirectX::XMFLOAT3 DirectionalLight::GetDirection() const
{
	return m_Direction;
}
