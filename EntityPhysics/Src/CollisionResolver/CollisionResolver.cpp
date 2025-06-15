#include "pch.h"
#include "CollisionResolver.h"

#include <algorithm>
#include <random>

void CollisionResolver::ResolveContact(Contact& contact, float deltaTime)
{
    ICollider* a = contact.Colliders[0];
    ICollider* b = contact.Colliders[1];
    if (!a || !b) return;

    if (IsStatic(a) && IsStatic(b))
    {
        return;
    }
    else if (a->GetColliderType() == ColliderType::Cube && b->GetColliderType() == ColliderType::Cube)
    {
        ResolveContactWithCubeVsCube(contact, deltaTime);
    }
}

void CollisionResolver::ResolveContacts(std::vector<Contact>& contacts, float deltaTime)
{
    for (Contact& contact : contacts)
    {
        ResolveContact(contact, deltaTime);
    }
}

void CollisionResolver::ResolveContactWithCubeVsCube(Contact& contact, float deltaTime)
{
    //ResolveVelocityWithCubeVsCube(contact, deltaTime);
    //ResolveFrictionWithCubeVsCube(contact, deltaTime);
    ResolvePenetration(contact, deltaTime);
    //ResolveAngularDampingWithCubeVsCube(contact, deltaTime);
}

void CollisionResolver::ResolvePenetration(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0];
    ICollider* colliderB = contact.Colliders[1];

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    // If both objects are static, do nothing
    if (isStaticA && isStaticB) return;

    const float slop = 0.01f;
    const float percent = 0.8f;
    float penetration = contact.PenetrationDepth - slop;
    if (penetration <= 0.0f) return;

    XMVECTOR normal = XMLoadFloat3(&contact.ContactNormal);

    if (isStaticA)
    {
        // Move only B along the normal
        XMVECTOR correction = XMVectorScale(normal, penetration * percent);
        XMVECTOR posB = bodyB->GetPosition();
        posB = XMVectorAdd(posB, correction);
        bodyB->SetPosition(posB);
    }
    else if (isStaticB)
    {
        // Move only A against the normal
        XMVECTOR correction = XMVectorScale(normal, penetration * percent);
        XMVECTOR posA = bodyA->GetPosition();
        posA = XMVectorSubtract(posA, correction);
        bodyA->SetPosition(posA);
    }
    else
    {
        // Both dynamic and split correction based on inverse mass
        float invMassA = bodyA->GetInverseMass();
        float invMassB = bodyB->GetInverseMass();
        float totalInvMass = invMassA + invMassB;

        if (totalInvMass <= 0.0f)
            return;

        XMVECTOR correction = XMVectorScale(normal, (penetration * percent) / totalInvMass);

        XMVECTOR posA = bodyA->GetPosition();
        posA = XMVectorSubtract(posA, XMVectorScale(correction, invMassA));
        bodyA->SetPosition(posA);

        XMVECTOR posB = bodyB->GetPosition();
        posB = XMVectorAdd(posB, XMVectorScale(correction, invMassB));
        bodyB->SetPosition(posB);
    }
}

void CollisionResolver::ResolveVelocityWithCubeVsCube(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0];
    ICollider* colliderB = contact.Colliders[1];

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    if (isStaticA && isStaticB)
        return;

    const float invMassA = bodyA->GetInverseMass();
    const float invMassB = bodyB->GetInverseMass();
    const float totalInvMass = invMassA + invMassB;

    if (totalInvMass <= 0.0f)
        return;

    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);
    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));

    // rA = contact - centerA
    XMVECTOR rA = XMVectorSubtract(contactPoint, bodyA->GetPosition());
    XMVECTOR rB = XMVectorSubtract(contactPoint, bodyB->GetPosition());

    // Velocity at contact points
    XMVECTOR vA = XMVectorAdd(bodyA->GetVelocity(), XMVector3Cross(bodyA->GetAngularVelocity(), rA));
    XMVECTOR vB = XMVectorAdd(bodyB->GetVelocity(), XMVector3Cross(bodyB->GetAngularVelocity(), rB));

    // Relative velocity
    XMVECTOR vRel = XMVectorSubtract(vA, vB);
    float vRelAlongNormal = XMVectorGetX(XMVector3Dot(vRel, normal));

    XMFLOAT3 relVelF;
    XMStoreFloat3(&relVelF, vRel);

    if (contact.PenetrationDepth <= 0.0f && vRelAlongNormal > 0.0f)
        return;

    // Compute combined restitution with elasticity
    float restitution = contact.Restitution * contact.Elasticity;

    // Compute angular terms
    XMVECTOR raCrossN = XMVector3Cross(rA, normal);
    XMVECTOR rbCrossN = XMVector3Cross(rB, normal);

    XMVECTOR raInertia = XMVector3Transform(raCrossN, bodyA->GetInverseInertiaTensorWorld());
    XMVECTOR rbInertia = XMVector3Transform(rbCrossN, bodyB->GetInverseInertiaTensorWorld());

    float angularTermA = XMVectorGetX(XMVector3Dot(raInertia, raCrossN));
    float angularTermB = XMVectorGetX(XMVector3Dot(rbInertia, rbCrossN));

    float denom = totalInvMass + angularTermA + angularTermB;

    if (denom <= 0.0f)
        return;

    float j = -(1.0f + restitution) * vRelAlongNormal / denom;
    contact.NormalImpulseMagnitude = j;

    XMVECTOR impulse = XMVectorScale(normal, j);

    // Apply impulses
    if (!isStaticA)
    {
        bodyA->ApplyLinearImpulse(impulse);
        bodyA->ApplyAngularImpulse(impulse, rA);
    }

    if (!isStaticB)
    {
        XMVECTOR negImpulse = XMVectorNegate(impulse);
        bodyB->ApplyLinearImpulse(negImpulse);
        bodyB->ApplyAngularImpulse(negImpulse, rB);
    }
}

void CollisionResolver::ResolveFrictionWithCubeVsCube(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    ICollider* colliderA = contact.Colliders[0];
    ICollider* colliderB = contact.Colliders[1];

    RigidBody* bodyA = colliderA->GetRigidBody();
    RigidBody* bodyB = colliderB->GetRigidBody();

    const bool isStaticA = colliderA->GetColliderState() == ColliderState::Static;
    const bool isStaticB = colliderB->GetColliderState() == ColliderState::Static;

    if (isStaticA && isStaticB)
        return;

    XMVECTOR contactPoint = XMLoadFloat3(&contact.ContactPoint);
    XMVECTOR normal = XMVector3Normalize(XMLoadFloat3(&contact.ContactNormal));

    // rA and rB
    XMVECTOR rA = XMVectorSubtract(contactPoint, bodyA->GetPosition());
    XMVECTOR rB = XMVectorSubtract(contactPoint, bodyB->GetPosition());

    // Velocity at contact point
    XMVECTOR vA = XMVectorAdd(bodyA->GetVelocity(), XMVector3Cross(bodyA->GetAngularVelocity(), rA));
    XMVECTOR vB = XMVectorAdd(bodyB->GetVelocity(), XMVector3Cross(bodyB->GetAngularVelocity(), rB));

    XMVECTOR vRel = XMVectorSubtract(vA, vB);

    // Remove normal component
    float vRelAlongNormal = XMVectorGetX(XMVector3Dot(vRel, normal));
    XMVECTOR vRelNormal = XMVectorScale(normal, vRelAlongNormal);
    XMVECTOR vTangent = XMVectorSubtract(vRel, vRelNormal);

    // Skip if no tangent movement
    if (XMVector3LengthSq(vTangent).m128_f32[0] < 1e-6f)
        return;

    XMVECTOR tangent = XMVector3Normalize(vTangent);

    // jt denominator
    XMVECTOR raCrossT = XMVector3Cross(rA, tangent);
    XMVECTOR rbCrossT = XMVector3Cross(rB, tangent);

    XMVECTOR raInertia = XMVector3Transform(raCrossT, bodyA->GetInverseInertiaTensorWorld());
    XMVECTOR rbInertia = XMVector3Transform(rbCrossT, bodyB->GetInverseInertiaTensorWorld());

    float angularTermA = XMVectorGetX(XMVector3Dot(raInertia, raCrossT));
    float angularTermB = XMVectorGetX(XMVector3Dot(rbInertia, rbCrossT));

    float denom = bodyA->GetInverseMass() + bodyB->GetInverseMass() + angularTermA + angularTermB;
    if (denom <= 0.0f)
        return;

    float jt = -XMVectorGetX(XMVector3Dot(vRel, tangent)) / denom;

    // Clamp friction impulse
    float maxFriction = contact.Friction * std::abs(contact.NormalImpulseMagnitude);
    jt = std::clamp(jt, -maxFriction, maxFriction);

    XMVECTOR frictionImpulse = XMVectorScale(tangent, jt);

    if (!isStaticA)
    {
        bodyA->ApplyLinearImpulse(frictionImpulse);
        bodyA->ApplyAngularImpulse(frictionImpulse, rA);
    }

    if (!isStaticB)
    {
        XMVECTOR negFrictionImpulse = XMVectorNegate(frictionImpulse);
        bodyB->ApplyLinearImpulse(negFrictionImpulse);
        bodyB->ApplyAngularImpulse(negFrictionImpulse, rB);
    }
}

void CollisionResolver::ResolveAngularDampingWithCubeVsCube(Contact& contact, float deltaTime)
{
    using namespace DirectX;

    for (int i = 0; i < 2; ++i)
    {
        ICollider* collider = contact.Colliders[i];
        if (!collider || collider->GetColliderState() == ColliderState::Static)
            continue;

        RigidBody* body = collider->GetRigidBody();
        if (!body)
            continue;

        XMVECTOR angVel = body->GetAngularVelocity();

        const float threshold = 0.01f;
        if (XMVectorGetX(XMVector3LengthSq(angVel)) < threshold)
        {
            body->SetAngularVelocity(XMVectorZero());
            continue;
        }

        float damping = body->GetAngularDamping();
        XMVECTOR damped = XMVectorScale(angVel, std::pow(damping, deltaTime));
        body->SetAngularVelocity(damped);
    }
}

bool CollisionResolver::IsStatic(const ICollider* collider)
{
    return collider->GetColliderState() == ColliderState::Static;
}
