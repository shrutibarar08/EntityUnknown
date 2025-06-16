#include "DirectionalLight.h"


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
	return
	{
		m_SpecularColor, m_AmbientColor, m_DiffuseColor, m_Direction, m_SpecularPower
	};
}

DirectX::XMFLOAT3 DirectionalLight::GetLightPosition() const
{
	//~ don't need to respond with distance.
	return {};
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
