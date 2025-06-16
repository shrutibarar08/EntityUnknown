#include "SpotLight.h"

void SpotLight::SetAmbient(float r, float g, float b, float a)
{
	m_AmbientColor = DirectX::XMFLOAT4(r, g, b, a);
}

void SpotLight::SetDiffuseColor(float r, float g, float b, float a)
{
	m_DiffuseColor = DirectX::XMFLOAT4(r, g, b, a);
}

void SpotLight::SetSpecularColor(float r, float g, float b, float a)
{
	m_SpecularColor = DirectX::XMFLOAT4(r, g, b, a);
}

void SpotLight::SetSpecularPower(float power)
{
	m_SpecularPower = power;
}

void SpotLight::SetPosition(float x, float y, float z)
{
	m_Position = DirectX::XMFLOAT3(x, y, z);
}

void SpotLight::SetDirection(float x, float y, float z)
{
	m_Direction = DirectX::XMFLOAT3(x, y, z);
}

void SpotLight::SetRange(float range)
{
	m_Range = range;
}

void SpotLight::SetSpotAngleDegrees(float degrees)
{
	m_SpotAngleDegree = degrees;
}

DirectX::XMFLOAT4 SpotLight::GetAmbientColor() const
{
	return m_AmbientColor;
}

DirectX::XMFLOAT4 SpotLight::GetDiffuseColor() const
{
	return m_DiffuseColor;
}

DirectX::XMFLOAT4 SpotLight::GetSpecularColor() const
{
	return m_SpecularColor;
}

DirectX::XMFLOAT3 SpotLight::GetPosition() const
{
	return m_Position;
}

DirectX::XMFLOAT3 SpotLight::GetDirection() const
{
	return m_Direction;
}

float SpotLight::GetRange() const
{
	return m_Range;
}

float SpotLight::GetSpotAngleDegree() const
{
	return m_SpotAngleDegree;
}

float SpotLight::GetSpecularPower() const
{
	return m_SpecularPower;
}

SPOT_LIGHT_GPU_DATA SpotLight::GetLightData() const
{
	SPOT_LIGHT_GPU_DATA data{};
	data.SpecularColor = m_SpecularColor;
	data.AmbientColor = m_AmbientColor;
	data.DiffuseColor = m_DiffuseColor;

	data.Position = m_Position;
	data.Range = m_Range;

	data.Direction = m_Direction;

	// Convert degree to cosine(radian) for GPU
	float radians = DirectX::XMConvertToRadians(m_SpotAngleDegree);
	data.SpotAngle = cosf(radians);

	data.SpecularPower = m_SpecularPower;
	return data;
}

DirectX::XMFLOAT3 SpotLight::GetLightPosition() const
{
	return m_Position;
}
