#pragma once
#include "ForceGenerator.h"
#include <DirectXMath.h>


class Gravity : public ForceGenerator
{
public:
    Gravity(const DirectX::XMVECTOR& g);
    void UpdateForce(ICollider* collider, float duration) override;

    bool IsGravityOn() const;
    void SetGravity(bool flag);
    void ReverseGravity();
    DirectX::XMVECTOR GetGravityForce() const;

private:
    bool m_Reversed{ false };
    bool m_GravityOn{ true };
    DirectX::XMVECTOR m_GravityForce;
};
