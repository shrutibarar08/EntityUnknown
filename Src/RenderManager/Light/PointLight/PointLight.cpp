#include "PointLight.h"
using namespace DirectX;

void PointLight::SetAmbient(float r, float g, float b, float a)
{
    m_AmbientColor = XMFLOAT4(r, g, b, a);
}

void PointLight::SetDiffuseColor(float r, float g, float b, float a)
{
    m_DiffuseColor = XMFLOAT4(r, g, b, a);
}

void PointLight::SetSpecularColor(float r, float g, float b, float a)
{
    m_SpecularColor = XMFLOAT4(r, g, b, a);
}

void PointLight::SetSpecularPower(float power)
{
    m_SpecularPower = power;
}

void PointLight::SetPosition(float x, float y, float z)
{
    m_Position = XMFLOAT3(x, y, z);
}

void PointLight::SetRange(float range)
{
    m_Range = range;
}

XMFLOAT4 PointLight::GetAmbientColor() const
{
	return m_AmbientColor;
}

XMFLOAT4 PointLight::GetDiffuseColor() const
{
	return m_DiffuseColor;
}

XMFLOAT4 PointLight::GetSpecularColor() const
{
	return m_SpecularColor;
}

XMFLOAT3 PointLight::GetPosition() const
{
	return m_Position;
}

float PointLight::GetRange() const
{
	return m_Range;
}

float PointLight::GetSpecularPower() const
{
	return m_SpecularPower;
}

POINT_LIGHT_GPU_DATA PointLight::GetLightData() const
{
    POINT_LIGHT_GPU_DATA data{};
    data.SpecularColor = m_SpecularColor;
    data.AmbientColor = m_AmbientColor;
    data.DiffuseColor = m_DiffuseColor;
    data.Position = m_Position;
    data.Range = m_Range;
    data.SpecularPower = m_SpecularPower;
    return data;
}

XMFLOAT3 PointLight::GetLightPosition() const
{
    return m_Position;
}
