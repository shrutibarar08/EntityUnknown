#pragma once

#include <array>
#include <DirectXMath.h>


class Frustum
{
public:
	Frustum()							= default;
	~Frustum()							= default;
	Frustum(const Frustum&)				= default;
	Frustum(Frustum&&)					= default;
	Frustum& operator=(const Frustum&)	= default;
	Frustum& operator=(Frustum&&)		= default;

	// Builds the frustum from a view-projection matrix (camera * projection)
	void ConstructFromMatrix(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, float screenDepth);

	bool IntersectsBoxCorners(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& halfSize) const;
	bool IntersectsAABB(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max) const;
	bool IntersectsSphereApprox(const DirectX::XMFLOAT3& center, float radius) const;

	const DirectX::XMFLOAT4& GetPlane(int index) const;
	static void NormalizePlane(DirectX::XMFLOAT4& plane);

	DirectX::XMFLOAT3 GetCenter() const;
	DirectX::XMFLOAT3 GetExtents() const;
	std::array<DirectX::XMFLOAT3, 8> GetCorners() const;

private:
	// 6 planes of the frustum
	DirectX::XMMATRIX m_ViewProjectionMatrix{};
	DirectX::XMFLOAT4 m_Planes[6];
};
