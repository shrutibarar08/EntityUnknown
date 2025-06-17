#include "IRender.h"

IRender::IRender()
{
	m_CubeCollider = std::make_unique<CubeCollider>(&m_RigidBody);
	m_bDirty = true;
}

void IRender::SetScreenWidth(int width)
{
	if (m_ScreenWidth != width) m_bDirty = true;

	m_ScreenWidth = width;
}

void IRender::SetScreenHeight(int height)
{
	if (m_ScreenHeight != height) m_bDirty = true;

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
	if (m_Scale.x != x || m_Scale.y != y || m_Scale.z == z) m_bDirty = true;
	m_Scale = { x, y, z };
}

void IRender::SetScale(const DirectX::XMFLOAT3& scale)
{
	if (m_Scale.x != scale.x || m_Scale.y != scale.y || m_Scale.z != scale.z)
		m_bDirty = true;
	m_Scale = scale;
}

void IRender::SetScale(const DirectX::XMVECTOR& scale)
{
	DirectX::XMFLOAT3 newScale;
	XMStoreFloat3(&newScale, scale);

	if (m_Scale.x != newScale.x || m_Scale.y != newScale.y || m_Scale.z != newScale.z)
		m_bDirty = true;
	m_Scale = newScale;
}

void IRender::SetScaleXY(float x, float y)
{
	if (m_Scale.x != x || m_Scale.y != y)
		m_bDirty = true;

	m_Scale.x = x;
	m_Scale.y = y;
}

void IRender::SetScaleXY(const DirectX::XMFLOAT2& scale)
{
	if (m_Scale.x != scale.x || m_Scale.y != scale.y)
		m_bDirty = true;

	m_Scale.x = scale.x;
	m_Scale.y = scale.y;
}

void IRender::SetScaleX(float x)
{
	if (m_Scale.x != x)
		m_bDirty = true;

	m_Scale.x = x;
}

void IRender::SetScaleY(float y)
{
	if (m_Scale.y != y)
		m_bDirty = true;

	m_Scale.y = y;
}

void IRender::SetScaleZ(float z)
{
	if (m_Scale.z != z)
		m_bDirty = true;

	m_Scale.z = z;
}

void IRender::AddScale(float x, float y, float z)
{
	if (x != 0.0f || y != 0.0f || z != 0.0f)
		m_bDirty = true;

	m_Scale.x += x;
	m_Scale.y += y;
	m_Scale.z += z;
}

void IRender::AddScale(const DirectX::XMFLOAT3& scale)
{
	if (scale.x != 0.0f || scale.y != 0.0f || scale.z != 0.0f)
		m_bDirty = true;

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
	m_bDirty = true;
}

void IRender::AddScale(float x, float y)
{
	m_Scale.x += x;
	m_Scale.y += y;
	m_bDirty = true;
}

void IRender::AddScaleXY(const DirectX::XMFLOAT2& scale)
{
	m_Scale.x += scale.x;
	m_Scale.y += scale.y;
	m_bDirty = true;
}

void IRender::AddScaleX(float x)
{
	m_Scale.x += x;
	m_bDirty = true;
}

void IRender::AddScaleY(float y)
{
	m_Scale.y += y;
	m_bDirty = true;
}

void IRender::AddScaleZ(float z)
{
	m_Scale.z += z;
	m_bDirty = true;
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
