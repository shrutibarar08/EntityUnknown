#include "pch.h"
#include "CubeCollider.h"
#include "Collision/Contact.h"

#include <algorithm>
#include <cmath>


CubeCollider::CubeCollider(RigidBody* body)
    : ICollider(body)
{
    DirectX::XMFLOAT3 scale; DirectX::XMStoreFloat3(&scale, m_Scale);
    m_RigidBody->ComputeInverseInertiaTensorBox(scale.x, scale.y, scale.z);
}

bool CubeCollider::CheckCollision(ICollider* other, Contact& outContact)
{
    if (!other) return false;
    if (m_ColliderState == ColliderState::Static && other->GetColliderState() == ColliderState::Static) return false;

    if (other->GetColliderType() == ColliderType::Cube)
    {
        Contact contact = GenerateContactWithCube(other->As<CubeCollider>());
        outContact = contact;

        return contact.PenetrationDepth > 0.0f;
    }
    return false;
}

ColliderType CubeCollider::GetColliderType() const
{
    return ColliderType::Cube;
}

void CubeCollider::SetScale(const DirectX::XMVECTOR& vector)
{
    m_Scale = vector;
    DirectX::XMFLOAT3 scale; DirectX::XMStoreFloat3(&scale, m_Scale);
    m_RigidBody->ComputeInverseInertiaTensorBox(scale.x, scale.y, scale.z);
}

DirectX::XMVECTOR CubeCollider::GetScale() const
{
    DirectX::XMVECTOR scale = m_Scale;
    return scale;
}

DirectX::XMVECTOR CubeCollider::GetHalfExtents() const
{
    DirectX::XMVECTOR scale = GetScale();
    return DirectX::XMVectorScale(scale, 0.5f);
}

void CubeCollider::ComputeWorldAxes(DirectX::XMVECTOR outAxes[3]) const
{
    using namespace DirectX;

    static const XMVECTOR LOCAL_AXES[3] = {
        XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), // X
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), // Y
        XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)  // Z
    };

    const Quaternion orientation = m_RigidBody->GetOrientation();
    const XMVECTOR halfScale = XMVectorScale(m_Scale, 1.0f); // Convert full size to half extents

    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR rotated = XMVector3Rotate(LOCAL_AXES[i], orientation.ToXmVector());

        // Scale by half extent along that axis
        float axisScale = XMVectorGetByIndex(halfScale, i); // Get half extent along x/y/z
        outAxes[i] = XMVectorScale(rotated, axisScale);
    }
}

DirectX::XMVECTOR CubeCollider::GetCenter() const
{
    return m_RigidBody->GetPosition();
}

DirectX::XMVECTOR CubeCollider::GetClosestPoint(DirectX::XMVECTOR point) const
{
    using namespace DirectX;

    // Step 1: Get the cube's world transform
    XMMATRIX world = GetWorldMatrix();
    XMMATRIX invWorld = XMMatrixInverse(nullptr, world);

    // Step 2: Transform the point into the cube's local space
    XMVECTOR localPoint = XMVector3TransformCoord(point, invWorld);

    // Step 3: Clamp the point to the cube's local AABB [-0.5, 0.5] in each axis
    XMVECTOR clamped = XMVectorClamp(
        localPoint,
        XMVectorSet(-0.5f, -0.5f, -0.5f, 0.0f),
        XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f)
    );

    // Step 4: Transform the clamped point back to world space
    XMVECTOR worldClosestPoint = XMVector3TransformCoord(clamped, world);
    return worldClosestPoint;
}

void CubeCollider::GetOBBAxes(const Quaternion& q, DirectX::XMVECTOR axes[3])
{
    using namespace DirectX;
    XMMATRIX rotMat = q.ToRotationMatrix();
    axes[0] = rotMat.r[0]; // Right (local X)
    axes[1] = rotMat.r[1]; // Up    (local Y)
    axes[2] = rotMat.r[2]; // Forward (local Z)
}

float CubeCollider::ProjectOBB(const DirectX::XMVECTOR& axis, const DirectX::XMVECTOR axes[3], const DirectX::XMVECTOR& halfExtents)
{
    using namespace DirectX;
    return fabsf(XMVectorGetX(XMVector3Dot(axes[0], axis))) * XMVectorGetX(halfExtents) +
        fabsf(XMVectorGetX(XMVector3Dot(axes[1], axis))) * XMVectorGetY(halfExtents) +
        fabsf(XMVectorGetX(XMVector3Dot(axes[2], axis))) * XMVectorGetZ(halfExtents);
}

bool CubeCollider::TryNormalize(DirectX::XMVECTOR& axis)
{
    using namespace DirectX;
    if (XMVector3LengthSq(axis).m128_f32[0] < 1e-6f) return false;
    axis = XMVector3Normalize(axis);
    return true;
}

void CubeCollider::BuildSATTestAxes(const DirectX::XMVECTOR axesA[3], const DirectX::XMVECTOR axesB[3],
    std::vector<DirectX::XMVECTOR>& outAxes)
{
    using namespace DirectX;
    outAxes.clear();
    outAxes.reserve(15);

    // === 3 Axes from A ===
    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR axis = XMVector3Normalize(axesA[i]);
        outAxes.push_back(axis);
    }

    // === 3 Axes from B ===
    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR axis = XMVector3Normalize(axesB[i]);
        outAxes.push_back(axis);
    }

    // === 9 Cross Product Axes ===
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            XMVECTOR cross = XMVector3Cross(axesA[i], axesB[j]);
            float lengthSq = XMVectorGetX(XMVector3LengthSq(cross));

            if (lengthSq > 1e-6f) // Ignore near-zero axes due to collinearity
            {
                outAxes.push_back(XMVector3Normalize(cross));
            }
        }
    }
}

bool CubeCollider::TestOBBsWithSAT(const CubeCollider* boxA, const CubeCollider* boxB,
    const std::vector<DirectX::XMVECTOR>& axes, float& minPenetrationDepth, DirectX::XMVECTOR& outCollisionNormal)
{
    using namespace DirectX;

    const XMVECTOR centerA = boxA->GetCenter();
    const XMVECTOR centerB = boxB->GetCenter();

    XMVECTOR axesA[3], axesB[3];
    boxA->ComputeWorldAxes(axesA);
    boxB->ComputeWorldAxes(axesB);

    const XMVECTOR centerOffset = XMVectorSubtract(centerB, centerA);

    minPenetrationDepth = FLT_MAX;
    outCollisionNormal = XMVectorZero();

    for (const auto& axis : axes)
    {
        if (XMVector3Equal(axis, XMVectorZero())) continue;

        XMVECTOR normalizedAxis = XMVector3Normalize(axis);

        // === Projection of box A onto axis ===
        float projectionA = 0.0f;
        for (int i = 0; i < 3; ++i)
        {
            projectionA += abs(XMVectorGetX(XMVector3Dot(normalizedAxis, axesA[i])));
        }

        // === Projection of box B onto axis ===
        float projectionB = 0.0f;
        for (int i = 0; i < 3; ++i)
        {
            projectionB += abs(XMVectorGetX(XMVector3Dot(normalizedAxis, axesB[i])));
        }

        // === Distance between centers on this axis ===
        float centerDistance = abs(XMVectorGetX(XMVector3Dot(normalizedAxis, centerOffset)));

        float totalProjection = projectionA + projectionB;

        if (centerDistance > totalProjection)
        {
            // Found a separating axis
            return false;
        }

        float overlap = totalProjection - centerDistance;
        if (overlap < minPenetrationDepth)
        {
            minPenetrationDepth = overlap;
            outCollisionNormal = normalizedAxis;

            // Ensure normal always points from A to B
            if (XMVectorGetX(XMVector3Dot(normalizedAxis, centerOffset)) < 0)
            {
                outCollisionNormal = XMVectorNegate(normalizedAxis);
            }
        }
    }

    return true;
}

bool CubeCollider::CheckCollisionWithCube(ICollider* other, Contact& outContact)
{
    using namespace DirectX;

    // === STEP 1: Type check & cast ===
    if (!other || other->GetColliderType() != ColliderType::Cube) return false;
    const CubeCollider* B = other->As<CubeCollider>();
    if (!B) return false;

    // === STEP 2: Get transforms and OBB axes ===
    const XMVECTOR posA = m_RigidBody->GetPosition();
    const XMVECTOR posB = B->GetRigidBody()->GetPosition();

    const XMVECTOR halfA = GetHalfExtents();
    const XMVECTOR halfB = B->GetHalfExtents();

    const Quaternion qA = m_RigidBody->GetOrientation();
    const Quaternion qB = B->GetRigidBody()->GetOrientation();

    XMVECTOR axesA[3];
    XMVECTOR axesB[3];

    GetOBBAxes(qA, axesA);
    GetOBBAxes(qB, axesB);

    const XMVECTOR toCenter = posB - posA;

    float minOverlap = FLT_MAX;
    XMVECTOR bestAxis = XMVectorZero();

    // === STEP 3: Test all 15 axes ===
    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR axis = axesA[i];
        if (!TryNormalize(axis)) continue;

        float projA = ProjectOBB(axis, axesA, halfA);
        float projB = ProjectOBB(axis, axesB, halfB);
        float dist = fabsf(XMVectorGetX(XMVector3Dot(toCenter, axis)));
        float overlap = projA + projB - dist;

        if (overlap < 0) return false; // Separating axis found

        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            bestAxis = axis;
            if (XMVectorGetX(XMVector3Dot(toCenter, axis)) < 0)
                bestAxis = -axis;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        XMVECTOR axis = axesB[i];
        if (!TryNormalize(axis)) continue;

        float projA = ProjectOBB(axis, axesA, halfA);
        float projB = ProjectOBB(axis, axesB, halfB);
        float dist = fabsf(XMVectorGetX(XMVector3Dot(toCenter, axis)));
        float overlap = projA + projB - dist;

        if (overlap < 0) return false;

        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            bestAxis = axis;
            if (XMVectorGetX(XMVector3Dot(toCenter, axis)) < 0)
                bestAxis = -axis;
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            XMVECTOR axis = XMVector3Cross(axesA[i], axesB[j]);
            if (!TryNormalize(axis)) continue;

            float projA = ProjectOBB(axis, axesA, halfA);
            float projB = ProjectOBB(axis, axesB, halfB);
            float dist = fabsf(XMVectorGetX(XMVector3Dot(toCenter, axis)));
            float overlap = projA + projB - dist;

            if (overlap < 0) return false;

            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                bestAxis = axis;
                if (XMVectorGetX(XMVector3Dot(toCenter, axis)) < 0)
                    bestAxis = -axis;
            }
        }
    }

    // === STEP 4: Fill out contact data ===
    outContact.Colliders[0] = this;
    outContact.Colliders[1] = other;

    XMStoreFloat3(&outContact.ContactNormal, bestAxis);
    outContact.PenetrationDepth = minOverlap;

    // Approximate center point between boxes (improve later via clipping or contact manifold)
    XMVECTOR midpoint = 0.5f * (posA + posB);
    XMStoreFloat3(&outContact.ContactPoint, midpoint);

    outContact.Restitution = 0.5f * (
        m_RigidBody->GetRestitution() + B->GetRigidBody()->GetRestitution());
    outContact.Friction = 0.5f * (
        m_RigidBody->GetFriction() + B->GetRigidBody()->GetFriction());
    outContact.Elasticity = 0.5f * (
        m_RigidBody->GetElasticity() + B->GetRigidBody()->GetElasticity());

    return true;
}

Contact CubeCollider::GenerateContactWithCube(CubeCollider* other)
{
    using namespace DirectX;

    Contact contact{};

    // Step 1: Compute axes
    XMVECTOR axesA[3];
    XMVECTOR axesB[3];
    this->ComputeWorldAxes(axesA);
    other->ComputeWorldAxes(axesB);

    // Step 2: Build SAT test axes
    std::vector<XMVECTOR> testAxes;
    BuildSATTestAxes(axesA, axesB, testAxes);

    // Step 3: Run SAT test
    float penetration;
    XMVECTOR normal;
    bool collided = TestOBBsWithSAT(this, other, testAxes, penetration, normal);

    if (!collided)
        return contact; // No collision

    // Step 4: Ensure normal points from A to B
    XMVECTOR centerA = this->GetCenter();
    XMVECTOR centerB = other->GetCenter();
    XMVECTOR centerDelta = XMVectorSubtract(centerB, centerA);

    if (XMVectorGetX(XMVector3Dot(centerDelta, normal)) < 0.0f)
    {
        normal = XMVectorNegate(normal);
    }

    // Step 5: Fill out contact
    XMStoreFloat3(&contact.ContactNormal, normal);
    contact.PenetrationDepth = penetration;

    // Approximate contact point: midpoint between centers
    XMVECTOR midPoint = XMVectorScale(XMVectorAdd(centerA, centerB), 0.5f);
    XMStoreFloat3(&contact.ContactPoint, midPoint);

    contact.Colliders[0] = this;
    contact.Colliders[1] = other;

    // Transfer physical properties
    contact.Restitution = Min(m_RigidBody->GetRestitution(), other->GetRigidBody()->GetRestitution());
    contact.Friction = std::sqrt(m_RigidBody->GetFriction() * other->GetRigidBody()->GetFriction());
    contact.Elasticity = Min(m_RigidBody->GetElasticity(), other->GetRigidBody()->GetElasticity());

    return contact;
}
