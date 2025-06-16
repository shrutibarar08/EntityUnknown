#include "IRender.h"

IRender::IRender()
{
	m_CubeCollider = std::make_unique<CubeCollider>(&m_RigidBody);
}

void IRender::SetScreenWidth(int width)
{
	m_ScreenWidth = width;;
}

void IRender::SetScreenHeight(int height)
{
	m_ScreenHeight = height;
}

CubeCollider* IRender::GetCubeCollider() const
{
	return m_CubeCollider.get();
}

RigidBody* IRender::GetRigidBody()
{
	return &m_RigidBody;
}

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

DirectX::XMMATRIX IRender::GetNormalTransform()
{
	using namespace DirectX;

	XMMATRIX scaleMat = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotMat = m_RigidBody.GetOrientation().ToRotationMatrix();

	XMMATRIX worldMat = scaleMat * rotMat;
	XMMATRIX normalMat = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMat));
	return normalMat;
}
