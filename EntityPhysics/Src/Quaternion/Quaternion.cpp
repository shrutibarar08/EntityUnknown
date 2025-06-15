#include "pch.h"
#include "Quaternion.h"
#include <cmath>

Quaternion::Quaternion() : R(1), I(0), J(0), K(0) {}
Quaternion::Quaternion(float r, float i, float j, float k)
    : R(r), I(i), J(j), K(k)
{
}

DirectX::XMVECTOR Quaternion::ToXmVector() const
{
    return DirectX::XMVectorSet(
        I.load(std::memory_order_relaxed),
        J.load(std::memory_order_relaxed),
        K.load(std::memory_order_relaxed),
        R.load(std::memory_order_relaxed)
    );
}

void Quaternion::Normalize()
{
    // Read all components atomically
    float r = R.load(std::memory_order_relaxed);
    float i = I.load(std::memory_order_relaxed);
    float j = J.load(std::memory_order_relaxed);
    float k = K.load(std::memory_order_relaxed);

    float d = r * r + i * i + j * j + k * k;

    if (d == 0.f)
    {
        R.store(1.0f, std::memory_order_relaxed);
        I.store(0.0f, std::memory_order_relaxed);
        J.store(0.0f, std::memory_order_relaxed);
        K.store(0.0f, std::memory_order_relaxed);
        return;
    }

    float inv = 1.0f / std::sqrt(d);

    R.store(r * inv, std::memory_order_relaxed);
    I.store(i * inv, std::memory_order_relaxed);
    J.store(j * inv, std::memory_order_relaxed);
    K.store(k * inv, std::memory_order_relaxed);
}

DirectX::XMVECTOR Quaternion::RotateVector(const DirectX::XMVECTOR& v) const
{
    using namespace DirectX;

    XMVECTOR q = XMVectorSet(
        I.load(std::memory_order_relaxed),
        J.load(std::memory_order_relaxed),
        K.load(std::memory_order_relaxed),
        R.load(std::memory_order_relaxed)
    );

    XMVECTOR qInv = XMQuaternionInverse(q);
    XMVECTOR vec = XMVectorSetW(v, 0.0f);

    XMVECTOR result = XMQuaternionMultiply(
        XMQuaternionMultiply(q, vec),
        qInv
    );

    return result;
}

Quaternion Quaternion::operator*(const Quaternion& q) const
{
    DirectX::XMVECTOR q1 = ToXmVector();     // This = [x, y, z, w]
    DirectX::XMVECTOR q2 = q.ToXmVector();   // q    = [x, y, z, w]

    // Multiply in standard quaternion order
    DirectX::XMVECTOR result = DirectX::XMQuaternionMultiply(q1, q2);

    // Convert the result vector back into a Quaternion object
    DirectX::XMFLOAT4 f;
    DirectX::XMStoreFloat4(&f, result);      // f = [x, y, z, w]
    return Quaternion(f.w, f.x, f.y, f.z);   // return Quaternion(w, x, y, z)
}

void Quaternion::AddScaledVector(const DirectX::XMVECTOR& vector, float scale)
{
    float r = R.load();
    float i = I.load();
    float j = J.load();
    float k = K.load();

    DirectX::XMVECTOR scaled = DirectX::XMVectorScale(vector, scale);
    float x = DirectX::XMVectorGetX(scaled);
    float y = DirectX::XMVectorGetY(scaled);
    float z = DirectX::XMVectorGetZ(scaled);

    // q = (0, x, y, z)
    float qr = -(i * x + j * y + k * z); // dot
    float qi = r * x + j * z - k * y;
    float qj = r * y + k * x - i * z;
    float qk = r * z + i * y - j * x;

    // Add scaled half to original
    R.store(r + 0.5f * qr);
    I.store(i + 0.5f * qi);
    J.store(j + 0.5f * qj);
    K.store(k + 0.5f * qk);
}

void Quaternion::RotateByVector(const DirectX::XMVECTOR& vector)
{
    float r = R.load();
    float i = I.load();
    float j = J.load();
    float k = K.load();

    float x = DirectX::XMVectorGetX(vector);
    float y = DirectX::XMVectorGetY(vector);
    float z = DirectX::XMVectorGetZ(vector);

    // q = this * Quaternion(0, x, y, z)
    float qr = -i * x - j * y - k * z;
    float qi = r * x + j * z - k * y;
    float qj = r * y + k * x - i * z;
    float qk = r * z + i * y - j * x;

    R.store(qr);
    I.store(qi);
    J.store(qj);
    K.store(qk);
}

Quaternion Quaternion::operator*(float scalar) const
{
    return Quaternion(
        R.load(std::memory_order_relaxed) * scalar,
        I.load(std::memory_order_relaxed) * scalar,
        J.load(std::memory_order_relaxed) * scalar,
        K.load(std::memory_order_relaxed) * scalar
    );
}


Quaternion& Quaternion::operator+=(const Quaternion& q)
{
    R.store(R.load(std::memory_order_relaxed) + q.R.load(std::memory_order_relaxed), std::memory_order_relaxed);
    I.store(I.load(std::memory_order_relaxed) + q.I.load(std::memory_order_relaxed), std::memory_order_relaxed);
    J.store(J.load(std::memory_order_relaxed) + q.J.load(std::memory_order_relaxed), std::memory_order_relaxed);
    K.store(K.load(std::memory_order_relaxed) + q.K.load(std::memory_order_relaxed), std::memory_order_relaxed);
    return *this;
}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
    if (this == &other) return *this;

    R.store(other.R.load(std::memory_order_relaxed), std::memory_order_relaxed);
    I.store(other.I.load(std::memory_order_relaxed), std::memory_order_relaxed);
    J.store(other.J.load(std::memory_order_relaxed), std::memory_order_relaxed);
    K.store(other.K.load(std::memory_order_relaxed), std::memory_order_relaxed);

    return *this;
}

Quaternion::Quaternion(const Quaternion& other)
{
    R.store(other.R.load(std::memory_order_relaxed), std::memory_order_relaxed);
    I.store(other.I.load(std::memory_order_relaxed), std::memory_order_relaxed);
    J.store(other.J.load(std::memory_order_relaxed), std::memory_order_relaxed);
    K.store(other.K.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

DirectX::XMMATRIX Quaternion::ToRotationMatrix() const
{
    using namespace DirectX;
    return XMMatrixRotationQuaternion(ToXmVector());
}

Quaternion operator*(float scalar, const Quaternion& q)
{
    return q * scalar;
}
