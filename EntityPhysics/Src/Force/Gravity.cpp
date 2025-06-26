#include "pch.h"
#include "Gravity.h"
#include "Collision/ICollider.h"
#include "RigidBody/RigidBody.h"

Gravity::Gravity(const DirectX::XMVECTOR& g)
    : m_GravityForce(g)
{
}

void Gravity::UpdateForce(ICollider* collider, float duration)
{
    //~ Pre checks
    if (!IsGravityOn() || !collider) return;
    RigidBody* rigidBody = collider->GetRigidBody();
    if (rigidBody->IsGrounded()) return;
    if (!rigidBody->HasFiniteMass() || collider->GetColliderState() == ColliderState::Static
        || collider->GetColliderState() == ColliderState::Trigger) return;

    //~ Thread safe access
    DirectX::XMVECTOR gravity = m_GravityForce;

    //~ apply it hehe
    float mass = rigidBody->GetMass();

    DirectX::XMVECTOR force = DirectX::XMVectorScale(gravity, mass);

    // Apply the gravitational force to the particle
    rigidBody->AddForce(force);
}

bool Gravity::IsGravityOn() const
{
    return m_GravityOn;
}

void Gravity::SetGravity(bool flag)
{
    m_GravityOn = flag;
}

void Gravity::ReverseGravity()
{
    using namespace DirectX;

    m_Reversed = !m_Reversed;

    XMFLOAT3 gravity{};
    XMStoreFloat3(&gravity, m_GravityForce);
    gravity.y = -gravity.y;
    m_GravityForce = XMLoadFloat3(&gravity);
}

DirectX::XMVECTOR Gravity::GetGravityForce() const
{
    return m_GravityForce;
}
