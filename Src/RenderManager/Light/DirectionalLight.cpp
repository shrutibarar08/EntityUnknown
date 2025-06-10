#include "DirectionalLight.h"


void DirectionalLight::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_DiffuseColor = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void DirectionalLight::SetDirection(float x, float y, float z)
{
	m_Direction = DirectX::XMFLOAT3(x, y, z);
}

DirectX::XMFLOAT4 DirectionalLight::GetDiffuseColor() const
{
	return m_DiffuseColor;
}

DirectX::XMFLOAT3 DirectionalLight::GetDirection() const
{
	return m_Direction;
}
