#pragma once
#include <DirectXMath.h>
#include "ICollider.h"

struct Contact
{
    DirectX::XMFLOAT3 ContactPoint;
    // Normal pointing from Body[0] to Body[1]
    DirectX::XMFLOAT3 ContactNormal;
    // How deep the objects are penetrating
    float PenetrationDepth = 0.0f;
    // Colliders involved
    ICollider* Colliders[2]{ nullptr, nullptr };
    // Physics response (to be used later)
    float Restitution = 1.0f;
    float Friction = 0.5f;
    float Elasticity = 1.0f;
    float NormalImpulseMagnitude = 0.0f;
};
