#include "IRender.h"

void IRender::SetScale(float x, float y, float z)
{
	m_Scale = { x, y, z };
}

void IRender::SetScale(const DirectX::XMFLOAT3& scale)
{
	m_Scale = scale;
}

void IRender::SetScale(const DirectX::XMVECTOR& scale)
{
	XMStoreFloat3(&m_Scale, scale);
}

void IRender::SetScaleXY(float x, float y)
{
	m_Scale.x = x;
	m_Scale.y = y;
}

void IRender::SetScaleXY(const DirectX::XMFLOAT2& scale)
{
	m_Scale.x = scale.x;
	m_Scale.y = scale.y;
}

void IRender::SetScaleX(float x)
{
	m_Scale.x = x;
}

void IRender::SetScaleY(float y)
{
	m_Scale.y = y;
}

void IRender::SetScaleZ(float z)
{
	m_Scale.z = z;
}

void IRender::AddScale(float x, float y, float z)
{
	m_Scale.x += x;
	m_Scale.y += y;
	m_Scale.z += z;
}

void IRender::AddScale(const DirectX::XMFLOAT3& scale)
{
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
}

void IRender::AddScale(float x, float y)
{
	m_Scale.x += x;
	m_Scale.y += y;
}

void IRender::AddScaleXY(const DirectX::XMFLOAT2& scale)
{
	m_Scale.x += scale.x;
	m_Scale.y += scale.y;
}

void IRender::AddScaleX(float x)
{
	m_Scale.x += x;
}

void IRender::AddScaleY(float y)
{
	m_Scale.y += y;
}

void IRender::AddScaleZ(float z)
{
	m_Scale.z += z;
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

void IRender::SetTranslation(float x, float y, float z)
{
	m_Translation = { x, y, z };
}

void IRender::SetTranslation(const DirectX::XMFLOAT3& pos)
{
	m_Translation = pos;
}

void IRender::SetTranslation(const DirectX::XMVECTOR& pos)
{
	XMStoreFloat3(&m_Translation, pos);
}

void IRender::SetTranslation(float x, float y)
{
	m_Translation.x = x;
	m_Translation.y = y;
}

void IRender::SetTranslationXY(const DirectX::XMFLOAT2& pos)
{
	m_Translation.x = pos.x;
	m_Translation.y = pos.y;
}

void IRender::SetTranslationX(float x)
{
	m_Translation.x = x;
}

void IRender::SetTranslationY(float y)
{
	m_Translation.y = y;
}

void IRender::SetTranslationZ(float z)
{
	m_Translation.z = z;
}

void IRender::AddTranslation(float x, float y, float z)
{
	m_Translation.x += x;
	m_Translation.y += y;
	m_Translation.z += z;
}

void IRender::AddTranslation(const DirectX::XMFLOAT3& pos)
{
	m_Translation.x += pos.x;
	m_Translation.y += pos.y;
	m_Translation.z += pos.z;
}

void IRender::AddTranslation(const DirectX::XMVECTOR& pos)
{
	DirectX::XMFLOAT3 temp;
	DirectX::XMStoreFloat3(&temp, pos);
	m_Translation.x += temp.x;
	m_Translation.y += temp.y;
	m_Translation.z += temp.z;
}

void IRender::AddTranslation(float x, float y)
{
	m_Translation.x += x;
	m_Translation.y += y;
}

void IRender::AddTranslationXY(const DirectX::XMFLOAT2& pos)
{
	m_Translation.x += pos.x;
	m_Translation.y += pos.y;
}

void IRender::AddTranslationX(float x)
{
	m_Translation.x += x;
}

void IRender::AddTranslationY(float y)
{
	m_Translation.y += y;
}

void IRender::AddTranslationZ(float z)
{
	m_Translation.z += z;
}

DirectX::XMFLOAT3 IRender::GetTranslation() const
{
	return m_Translation;
}

DirectX::XMFLOAT2 IRender::GetTranslationXY() const
{
	return { m_Translation.x, m_Translation.y };
}

float IRender::GetPositionX() const
{
	return m_Translation.x;
}

float IRender::GetPositionY() const
{
	return m_Translation.y;
}

float IRender::GetPositionZ() const
{
	return m_Translation.z;
}

void IRender::SetRotation(float pitch, float yaw, float roll)
{
	m_Rotation = { pitch, yaw, roll };
}

void IRender::SetRotation(const DirectX::XMFLOAT3& rot)
{
	m_Rotation = rot;
}

void IRender::SetRotation(const DirectX::XMVECTOR& rot)
{
	XMStoreFloat3(&m_Rotation, rot);
}

void IRender::SetYaw(float yaw)
{
	m_Rotation.y = yaw;
}

void IRender::SetPitch(float pitch)
{
	m_Rotation.x = pitch;
}

void IRender::SetRoll(float roll)
{
	m_Rotation.z = roll;
}

void IRender::AddRotation(float pitch, float yaw, float roll)
{
	m_Rotation.x += pitch;
	m_Rotation.y += yaw;
	m_Rotation.z += roll;
}

void IRender::AddRotation(const DirectX::XMFLOAT3& rot)
{
	m_Rotation.x += rot.x;
	m_Rotation.y += rot.y;
	m_Rotation.z += rot.z;
}

void IRender::AddRotation(const DirectX::XMVECTOR& rot)
{
	DirectX::XMFLOAT3 temp;
	DirectX::XMStoreFloat3(&temp, rot);
	m_Rotation.x += temp.x;
	m_Rotation.y += temp.y;
	m_Rotation.z += temp.z;
}

void IRender::AddYaw(float yaw)
{
	m_Rotation.y += yaw;
}

void IRender::AddPitch(float pitch)
{
	m_Rotation.x += pitch;
}

void IRender::AddRoll(float roll)
{
	m_Rotation.z += roll;
}

DirectX::XMFLOAT3 IRender::GetRotation() const
{
	return m_Rotation;
}

float IRender::GetYaw() const
{
	return m_Rotation.y;
}

float IRender::GetPitch() const
{
	return m_Rotation.x;
}

float IRender::GetRoll() const
{
	return m_Rotation.z;
}

DirectX::XMMATRIX IRender::GetNormalTransform() const
{
	using namespace DirectX;

	XMMATRIX scaleMat = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(GetPitch(), GetYaw(), GetRoll());
	XMMATRIX normalMat = XMMatrixTranspose(XMMatrixInverse(nullptr, scaleMat * rotMat));
	return normalMat;
}
