#include "Frustum.h"

#include <ranges>
#include <cmath>

#include "Utils/Logger/Logger.h"


void Frustum::ConstructFromMatrix(
    const DirectX::XMMATRIX& viewMatrix,
    const DirectX::XMMATRIX& projectionMatrix,
    float screenDepth)
{
    using namespace DirectX;

    m_ViewProjectionMatrix = viewMatrix * projectionMatrix;

    // Make a mutable copy of the projection matrix
    XMFLOAT4X4 projFloat4x4;
    XMStoreFloat4x4(&projFloat4x4, projectionMatrix);

    float zMin = -projFloat4x4._43 / projFloat4x4._33;
    float r = screenDepth / (screenDepth - zMin);
    projFloat4x4._33 = r;
    projFloat4x4._43 = -r * zMin;

    XMMATRIX adjustedProj = XMLoadFloat4x4(&projFloat4x4);
    XMMATRIX viewProj = XMMatrixMultiply(viewMatrix, adjustedProj);

    // Now extract planes
    XMFLOAT4X4 matrix;
    XMStoreFloat4x4(&matrix, viewProj);

    auto Normalize = [](XMFLOAT4& p)
        {
            float length = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            p.x /= length; p.y /= length; p.z /= length; p.w /= length;
        };

    // Near
    m_Planes[0] = { matrix._13, matrix._23, matrix._33, matrix._43 };
    Normalize(m_Planes[0]);

    // Far
    m_Planes[1] = { matrix._14 - matrix._13, matrix._24 - matrix._23, matrix._34 - matrix._33, matrix._44 - matrix._43 };
    Normalize(m_Planes[1]);

    // Left
    m_Planes[2] = { matrix._14 + matrix._11, matrix._24 + matrix._21, matrix._34 + matrix._31, matrix._44 + matrix._41 };
    Normalize(m_Planes[2]);

    // Right
    m_Planes[3] = { matrix._14 - matrix._11, matrix._24 - matrix._21, matrix._34 - matrix._31, matrix._44 - matrix._41 };
    Normalize(m_Planes[3]);

    // Top
    m_Planes[4] = { matrix._14 - matrix._12, matrix._24 - matrix._22, matrix._34 - matrix._32, matrix._44 - matrix._42 };
    Normalize(m_Planes[4]);

    // Bottom
    m_Planes[5] = { matrix._14 + matrix._12, matrix._24 + matrix._22, matrix._34 + matrix._32, matrix._44 + matrix._42 };
    Normalize(m_Planes[5]);
}

bool Frustum::IntersectsBoxCorners(
    const DirectX::XMFLOAT3& center,
    const DirectX::XMFLOAT3& halfSize) const
{
    using namespace DirectX;

    for (const XMFLOAT4& plane : m_Planes)
    {
        bool anyInside = false;

        // Test all 8 corners
        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2)
                {
                    XMFLOAT3 corner =
                    {
                        center.x + x * halfSize.x,
                        center.y + y * halfSize.y,
                        center.z + z * halfSize.z
                    };

                    float distance =
                        plane.x * corner.x +
                        plane.y * corner.y +
                        plane.z * corner.z +
                        plane.w;

                    if (distance >= 0.0f)
                    {
                        anyInside = true;
                        break;
                    }
                }

        if (!anyInside)
            return false; // All corners outside this plane
    }

    return true; // Passed all planes
}

bool Frustum::IntersectsAABB(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max) const
{
    for (const DirectX::XMFLOAT4& plane : m_Planes)
    {
        int outside = 0;
        for (int x = 0; x <= 1; ++x)
            for (int y = 0; y <= 1; ++y)
                for (int z = 0; z <= 1; ++z)
                {
	                DirectX::XMFLOAT3 corner =
                    {
                        x ? max.x : min.x,
                        y ? max.y : min.y,
                        z ? max.z : min.z
                    };

                    float d = plane.x * corner.x + plane.y * corner.y + plane.z * corner.z + plane.w;
                    if (d < 0.0f) ++outside;
                }

        if (outside == 8) return false; // Completely outside
    }
    return true; // At least partially inside
}

bool Frustum::IntersectsSphereApprox(const DirectX::XMFLOAT3& center, float radius) const
{
    for (const auto& plane : m_Planes)
    {
        float d = plane.x * center.x + plane.y * center.y + plane.z * center.z + plane.w;
        if (d < -radius) return false;
    }
    return true;
}

const DirectX::XMFLOAT4& Frustum::GetPlane(int index) const
{
	return m_Planes[index];
}

void Frustum::NormalizePlane(DirectX::XMFLOAT4& plane)
{
	DirectX::XMVECTOR v = XMLoadFloat4(&plane);
    DirectX::XMVECTOR n = DirectX::XMVector3Normalize(v); // normalize (x, y, z)
    XMStoreFloat4(&plane, n);
}

DirectX::XMFLOAT3 Frustum::GetCenter() const
{
    auto corners = GetCorners();

    DirectX::XMVECTOR sum = DirectX::XMVectorZero();
    for (const auto& corner : corners)
    {
        sum = DirectX::XMVectorAdd(sum, XMLoadFloat3(&corner));
    }

    DirectX::XMVECTOR center = DirectX::XMVectorScale(sum, 1.0f / static_cast<float>(corners.size()));
    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, center);
    return result;
}

DirectX::XMFLOAT3 Frustum::GetExtents() const
{
    auto corners = GetCorners();

    DirectX::XMVECTOR minV = XMLoadFloat3(&corners[0]);
    DirectX::XMVECTOR maxV = XMLoadFloat3(&corners[0]);

    for (size_t i = 1; i < corners.size(); ++i)
    {
	    DirectX::XMVECTOR corner = XMLoadFloat3(&corners[i]);
        minV = DirectX::XMVectorMin(minV, corner);
        maxV = DirectX::XMVectorMax(maxV, corner);
    }

    DirectX::XMVECTOR extents = DirectX::XMVectorScale(DirectX::XMVectorSubtract(maxV, minV), 0.5f);

    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, extents);
    return result;
}


std::array<DirectX::XMFLOAT3, 8> Frustum::GetCorners() const
{
    using namespace DirectX;

    std::array<XMFLOAT3, 8> corners;

    // Frustum corners in NDC space [-1, 1]
    static const XMFLOAT4 ndcCorners[8] =
    {
        {-1.0f,  1.0f, 0.0f, 1.0f}, // near top left
        { 1.0f,  1.0f, 0.0f, 1.0f}, // near top right
        { 1.0f, -1.0f, 0.0f, 1.0f}, // near bottom right
        {-1.0f, -1.0f, 0.0f, 1.0f}, // near bottom left
        {-1.0f,  1.0f, 1.0f, 1.0f}, // far top left
        { 1.0f,  1.0f, 1.0f, 1.0f}, // far top right
        { 1.0f, -1.0f, 1.0f, 1.0f}, // far bottom right
        {-1.0f, -1.0f, 1.0f, 1.0f}, // far bottom left
    };

    XMMATRIX invVP = XMMatrixInverse(nullptr, m_ViewProjectionMatrix);

    for (int i = 0; i < 8; ++i)
    {
        XMVECTOR corner = XMLoadFloat4(&ndcCorners[i]);
        XMVECTOR worldPos = XMVector4Transform(corner, invVP);
        worldPos = XMVectorScale(worldPos, 1.0f / XMVectorGetW(worldPos)); // perspective divide
        XMStoreFloat3(&corners[i], worldPos);
    }

    return corners;
}
