#include "pch.h"
#include "ICollider.h"

ICollider::ICollider(RigidBody* attachBody)
	: m_RigidBody(attachBody)
{}

ColliderState ICollider::GetColliderState() const
{
	return m_ColliderState;
}

void ICollider::SetColliderState(ColliderState state)
{
	m_ColliderState = state;
}

const char* ICollider::ToString() const
{
	switch (GetColliderType())
	{
	case ColliderType::Cube:    return "Cube";
	default:                    return "Unknown";
	}
	return "null";
}

const char* ICollider::GetColliderTypeName() const
{
	switch (GetColliderType())
	{
	case ColliderType::Cube: return "Cube";
	default: return "Unknown";
	}
}

DirectX::XMMATRIX ICollider::GetTransformationMatrix() const
{
	return m_TransformationMatrix;
}

void ICollider::Update(float deltaTime)
{
	m_TransformationMatrix =
		DirectX::XMMatrixScalingFromVector(GetScale()) *
		DirectX::XMMatrixRotationQuaternion(m_RigidBody->GetOrientation().ToXmVector()) *
		DirectX::XMMatrixTranslationFromVector(m_RigidBody->GetPosition());

}

DirectX::XMMATRIX ICollider::GetWorldMatrix() const
{
	using namespace DirectX;
	XMVECTOR scale = GetScale();
	XMVECTOR rotationQuat = m_RigidBody->GetOrientation().ToXmVector();
	XMVECTOR position = m_RigidBody->GetPosition();

	return XMMatrixAffineTransformation(
		scale,                     // Scaling
		XMVectorZero(),            // Rotation origin
		rotationQuat,              // Rotation quaternion
		position                   // Translation
	);
}
