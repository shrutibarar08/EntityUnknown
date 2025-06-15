#pragma once
#include <vector>
#include "Collision/ICollider.h"


class CubeCollider final : public ICollider
{
public:
	CubeCollider(RigidBody* body);
	~CubeCollider() override = default;

	CubeCollider(const CubeCollider&) = default;
	CubeCollider(CubeCollider&&) = default;
	CubeCollider& operator=(const CubeCollider&) = default;
	CubeCollider& operator=(CubeCollider&&) = default;

	//~ Collision Interface
	bool CheckCollision(ICollider* other, Contact& outContact) override;
	ColliderType GetColliderType() const override;
	void SetScale(const DirectX::XMVECTOR& vector) override;
	DirectX::XMVECTOR GetScale() const override;

	//~ Cube Collision Specifics
	DirectX::XMVECTOR GetHalfExtents() const;
	void ComputeWorldAxes(DirectX::XMVECTOR outAxes[3]) const;
	DirectX::XMVECTOR GetCenter() const;
	DirectX::XMVECTOR GetClosestPoint(DirectX::XMVECTOR point) const;

	//~ Helpers
	static void GetOBBAxes(const Quaternion& q, DirectX::XMVECTOR axes[3]);
	static float ProjectOBB(const DirectX::XMVECTOR& axis, const DirectX::XMVECTOR axes[3], const DirectX::XMVECTOR& halfExtents);
	static bool TryNormalize(DirectX::XMVECTOR& axis);
	static void BuildSATTestAxes(const DirectX::XMVECTOR axesA[3],
		const DirectX::XMVECTOR axesB[3],
		std::vector<DirectX::XMVECTOR>& outAxes);
	static bool TestOBBsWithSAT(
		const CubeCollider* boxA,
		const CubeCollider* boxB,
		const std::vector<DirectX::XMVECTOR>& axes,
		float& minPenetrationDepth,
		DirectX::XMVECTOR& outCollisionNormal
	);

private:
	//~ Cube Collision Check
	bool CheckCollisionWithCube(ICollider* other, Contact& outContact);
	Contact GenerateContactWithCube(CubeCollider* other);

private:
	DirectX::XMVECTOR m_Scale{ 1.0f, 1.0f, 1.0f };
};
