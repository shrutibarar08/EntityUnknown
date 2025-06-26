#include "pch.h"
#include "ICollider.h"

#include <iostream>

#include "Contact.h"
#include <ranges>

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

void ICollider::RegisterCollisionEffect(ICollider* other, const Contact& contact)
{
	const bool thisIsDynamic = this->GetColliderState() == ColliderState::Dynamic;
	const bool otherIsStatic = other->GetColliderState() == ColliderState::Static;

	const bool thisIsStatic = this->GetColliderState() == ColliderState::Static;
	const bool otherIsDynamic = other->GetColliderState() == ColliderState::Dynamic;

	const float normalY = contact.ContactNormal.y;
	const float normalX = contact.ContactNormal.x;

	if (thisIsDynamic && otherIsStatic)
	{
		auto velocity = m_RigidBody->GetVelocity();

		// === Grounded check (landing)
		if (normalY < -0.7f)
		{
			ResetFallCoolDown();
			m_RigidBody->SetGrounded(true);
			velocity = DirectX::XMVectorSetY(velocity, 0.0f);
		}

		// === X-collision check (horizontal block)
		if (std::abs(normalX) > 0.7f)
		{
			velocity = DirectX::XMVectorSetX(velocity, 0.0f);
		}

		m_RigidBody->SetVelocity(velocity);
	}
	else if (thisIsStatic && otherIsDynamic)
	{
		auto velocity = other->GetRigidBody()->GetVelocity();

		// === Grounded check (other lands on us)
		if (normalY > 0.7f)
		{
			other->ResetFallCoolDown();
			other->GetRigidBody()->SetGrounded(true);
			velocity = DirectX::XMVectorSetY(velocity, 0.0f);
		}

		// === X-collision check (we're blocking other)
		if (std::abs(normalX) > 0.7f)
		{
			velocity = DirectX::XMVectorSetX(velocity, 0.0f);
		}

		other->GetRigidBody()->SetVelocity(velocity);
	}
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
	m_FallCoolDown -= deltaTime;

	if (m_FallCoolDown <= 0.0f && GetColliderState() == ColliderState::Dynamic)
	{
		GetRigidBody()->SetGrounded(false);
	} 

	m_TransformationMatrix =
		DirectX::XMMatrixScalingFromVector(GetScale()) *
		DirectX::XMMatrixRotationQuaternion(m_RigidBody->GetOrientation().ToXmVector()) *
		DirectX::XMMatrixTranslationFromVector(m_RigidBody->GetPosition());

	for (auto& [collider, info] : m_CollidersInfo)
	{
		if (info.HasEntered)
		{
			Contact filler;
			if (!CheckCollision(collider, filler))
			{
				if (info.m_OnTriggerExitCallbackFn) info.m_OnTriggerExitCallbackFn();
				info.HasEntered = false;
			}
		}
	}
}

void ICollider::SetTriggerTarget(const TRIGGER_COLLISION_INFO& triggerCollisionInfo)
{
	if (!m_CollidersInfo.contains(triggerCollisionInfo.TargetCollider))
	{
		ColliderInfo info{};
		info.HasExited = false;
		info.HasEntered = false;
		info.m_OnTriggerEnterCallbackFn = triggerCollisionInfo.m_OnTriggerEnterCallbackFn;
		info.m_OnTriggerExitCallbackFn = triggerCollisionInfo.m_OnTriggerExitCallbackFn;
		m_CollidersInfo[triggerCollisionInfo.TargetCollider] = std::move(info);
	}
}


void ICollider::RegisterCollision(ICollider* other, const Contact& contact)
{
	// === Skip trigger vs trigger ===
	if (other->GetColliderState() == ColliderState::Trigger &&
		m_ColliderState == ColliderState::Trigger)
		return;

	RegisterCollisionEffect(other, contact);

	// === Let the trigger handle it ===
	if (other->GetColliderState() == ColliderState::Trigger)
	{
		other->RegisterCollision(this, contact);
		return;
	}

	// === This is the trigger ===
	if (m_ColliderState == ColliderState::Trigger)
	{
		auto it = m_CollidersInfo.find(other);
		if (it != m_CollidersInfo.end())
		{
			ColliderInfo& info = it->second;

			if (!info.HasEntered)
			{
				info.HasEntered = true;
				info.HasExited = false;

				if (info.m_OnTriggerEnterCallbackFn)
					info.m_OnTriggerEnterCallbackFn();
			}
		}
		return; // prevent fallthrough to rigid body logic
	}
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
