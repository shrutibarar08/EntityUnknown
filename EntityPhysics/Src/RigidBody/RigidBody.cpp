#include "pch.h"
#include "RigidBody.h"
#include <cmath>

RigidBody::RigidBody()
    :
    InverseMass(1.0f),
    Orientation(1, 0, 0, 0)
{
    m_InverseInertiaTensorLocal = DirectX::XMMatrixIdentity();
    CalculateDerivedData();
}

void RigidBody::AddForce(const DirectX::XMVECTOR& force)
{
    DirectX::XMVECTOR current = ForceAccum.Get();
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, force);
    ForceAccum.Set(updated);
}

void RigidBody::AddTorque(const DirectX::XMVECTOR& torque)
{
    DirectX::XMVECTOR current = TorqueAccum.Get();
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, torque);
    TorqueAccum.Set(updated);
}

void RigidBody::Integrate(float dt, IntegrationType type)
{
    switch (type)
    {
    case IntegrationType::Euler:
        IntegrateEuler(dt);
        break;
    case IntegrationType::SemiImplicitEuler:
        IntegrateSemiImplicitEuler(dt);
        break;
    case IntegrationType::Verlet:
        IntegrateVerlet(dt);
        break;
    }
}

DirectX::XMMATRIX RigidBody::GetTransformMatrix()
{
    using namespace DirectX;
    XMMATRIX rotation = Orientation.ToRotationMatrix();
    XMMATRIX translation = XMMatrixTranslationFromVector(Position.Get());
    return rotation * translation;
}

void RigidBody::CalculateDerivedData()
{
    using namespace DirectX;

    Orientation.Normalize(); // Prevent drift

    XMMATRIX rotMatrix = XMMatrixRotationQuaternion(Orientation.ToXmVector());
    XMMATRIX translation = XMMatrixTranslationFromVector(Position.Get());

    TransformMatrix = rotMatrix * translation; // Full local-to-world matrix

    XMMATRIX rotTranspose = XMMatrixTranspose(rotMatrix);
    InverseInertiaTensorWorld = XMMatrixMultiply(
        XMMatrixMultiply(rotMatrix, m_InverseInertiaTensorLocal),
        rotTranspose
    );
}
void RigidBody::ClearAccumulators()
{
    ForceAccum.Set(DirectX::XMVectorZero());
    TorqueAccum.Set(DirectX::XMVectorZero());
}

// Setters
void RigidBody::SetPosition(const DirectX::XMVECTOR& pos)
{
    Position.Set(pos);
    m_VerletNeedsReset = true;
}

void RigidBody::SetTranslation(float x, float y, float z)
{
    DirectX::XMVECTOR vector{ x, y, z };
    SetPosition(vector);
}

void RigidBody::SetTranslation(const DirectX::XMFLOAT3& pos)
{
    DirectX::XMVECTOR vector{ pos.x, pos.y, pos.z};
    SetPosition(vector);
}

void RigidBody::SetTranslation(const DirectX::XMVECTOR& pos)
{
    SetPosition(pos);
}

void RigidBody::SetTranslation(float x, float y)
{
    DirectX::XMVECTOR vector{ x, y, 0.0f};
    SetPosition(vector);
}

void RigidBody::SetTranslationXY(const DirectX::XMFLOAT2& pos)
{
    DirectX::XMVECTOR current = Position.Get();

    // Overwrite X and Y, keep Z and W the same
    DirectX::XMVECTOR updated = DirectX::XMVectorSet(
        pos.x,
        pos.y,
        DirectX::XMVectorGetZ(current),
        DirectX::XMVectorGetW(current)
    );

    Position.Set(updated);
}

void RigidBody::SetTranslationX(float x)
{
    DirectX::XMVECTOR current = Position.Get();

    // Replace X, keep Y, Z, W
    DirectX::XMVECTOR updated = DirectX::XMVectorSetX(current, x);

    Position.Set(updated);
}

void RigidBody::SetTranslationY(float y)
{
    DirectX::XMVECTOR current = Position.Get();

    // Replace Y, keep X, Z, W
    DirectX::XMVECTOR updated = DirectX::XMVectorSetY(current, y);

    Position.Set(updated);
}

void RigidBody::SetTranslationZ(float z)
{
    DirectX::XMVECTOR current = Position.Get();

    // Replace Z, keep X, Y, W
    DirectX::XMVECTOR updated = DirectX::XMVectorSetZ(current, z);

    Position.Set(updated);
}

void RigidBody::AddTranslation(float x, float y, float z)
{
    DirectX::XMVECTOR current = Position.Get();
    DirectX::XMVECTOR delta = DirectX::XMVectorSet(x, y, z, 0.0f);
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, delta);

    Position.Set(updated);
}

void RigidBody::AddTranslation(const DirectX::XMFLOAT3& pos)
{
    // Convert XMFLOAT3 to XMVECTOR
    DirectX::XMVECTOR delta = DirectX::XMLoadFloat3(&pos);

    AddTranslation(delta); // Reuse the XMVECTOR overload
}

void RigidBody::AddTranslation(const DirectX::XMVECTOR& pos)
{
    DirectX::XMVECTOR current = Position.Get();

    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, pos);

    Position.Set(updated);
}

void RigidBody::AddTranslation(float x, float y)
{
    DirectX::XMVECTOR current = Position.Get();
    // Create delta vector: X, Y, Z=0, W=0
    DirectX::XMVECTOR delta = DirectX::XMVectorSet(x, y, 0.0f, 0.0f);
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, delta);

    Position.Set(updated);
}

void RigidBody::AddTranslationXY(const DirectX::XMFLOAT2& pos)
{
    AddTranslation(pos.x, pos.y); // Reuse the overload
}

void RigidBody::AddTranslationX(float x)
{
    DirectX::XMVECTOR current = Position.Get();
    DirectX::XMVECTOR delta = DirectX::XMVectorSet(x, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, delta);
    Position.Set(updated);
}

void RigidBody::AddTranslationY(float y)
{
    DirectX::XMVECTOR current = Position.Get();

    // Delta only in Y direction
    DirectX::XMVECTOR delta = DirectX::XMVectorSet(0.0f, y, 0.0f, 0.0f);
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, delta);

    Position.Set(updated);
}

void RigidBody::AddTranslationZ(float z)
{
    DirectX::XMVECTOR current = Position.Get();
    // Delta only in Z direction
    DirectX::XMVECTOR delta = DirectX::XMVectorSet(0.0f, 0.0f, z, 0.0f);
    DirectX::XMVECTOR updated = DirectX::XMVectorAdd(current, delta);

    Position.Set(updated);
}

DirectX::XMFLOAT3 RigidBody::GetTranslation()
{
    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, Position.Get());
    return result;
}

DirectX::XMFLOAT2 RigidBody::GetTranslationXY()
{
    DirectX::XMFLOAT2 result;
    DirectX::XMVECTOR pos = Position.Get();
    result.x = DirectX::XMVectorGetX(pos);
    result.y = DirectX::XMVectorGetY(pos);
    return result;
}

float RigidBody::GetPositionX()
{
    return DirectX::XMVectorGetX(Position.Get());
}

float RigidBody::GetPositionY()
{
    return DirectX::XMVectorGetY(Position.Get());
}

float RigidBody::GetPositionZ()
{
    return DirectX::XMVectorGetZ(Position.Get());
}

void RigidBody::SetRotation(float pitch, float yaw, float roll)
{
    DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

    Orientation = Quaternion(
        DirectX::XMVectorGetW(quat),
        DirectX::XMVectorGetX(quat),
        DirectX::XMVectorGetY(quat),
        DirectX::XMVectorGetZ(quat)
    );
}

void RigidBody::SetRotation(const DirectX::XMFLOAT3& rot)
{
    SetRotation(rot.x, rot.y, rot.z); // Reuse existing overload
}

void RigidBody::SetRotation(const DirectX::XMVECTOR& rot)
{
    Orientation = Quaternion(
        DirectX::XMVectorGetW(rot),
        DirectX::XMVectorGetX(rot),
        DirectX::XMVectorGetY(rot),
        DirectX::XMVectorGetZ(rot)
    );
}

void RigidBody::SetYaw(float yaw)
{
    DirectX::XMFLOAT3 euler = QuaternionToEuler(Orientation);
    SetRotation(euler.x, yaw, euler.z); // pitch, yaw, roll
}

void RigidBody::SetPitch(float pitch)
{
    DirectX::XMFLOAT3 euler = QuaternionToEuler(Orientation);
    SetRotation(pitch, euler.y, euler.z); // pitch, yaw, roll
}

void RigidBody::SetRoll(float roll)
{
    DirectX::XMFLOAT3 euler = QuaternionToEuler(Orientation);
    SetRotation(euler.x, euler.y, roll); // pitch, yaw, roll
}

void RigidBody::AddRotation(float pitch, float yaw, float roll)
{
    // Build delta quaternion from Euler
    DirectX::XMVECTOR qDelta = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

    // Compose with existing orientation
    DirectX::XMVECTOR qCurrent = Orientation.ToXmVector();
    DirectX::XMVECTOR qResult = DirectX::XMQuaternionMultiply(qDelta, qCurrent);

    Orientation = Quaternion(
        DirectX::XMVectorGetW(qResult),
        DirectX::XMVectorGetX(qResult),
        DirectX::XMVectorGetY(qResult),
        DirectX::XMVectorGetZ(qResult)
    );
}

void RigidBody::AddRotation(const DirectX::XMFLOAT3& rot)
{
    AddRotation(rot.x, rot.y, rot.z); // Reuse float overload
}

void RigidBody::AddRotation(const DirectX::XMVECTOR& rot)
{
    // Compose: qResult = rot * current
    DirectX::XMVECTOR qCurrent = Orientation.ToXmVector();
    DirectX::XMVECTOR qResult = DirectX::XMQuaternionMultiply(rot, qCurrent);

    Orientation = Quaternion(
        DirectX::XMVectorGetW(qResult),
        DirectX::XMVectorGetX(qResult),
        DirectX::XMVectorGetY(qResult),
        DirectX::XMVectorGetZ(qResult)
    );
}

void RigidBody::AddYaw(float yaw)
{
    // Delta quaternion for yaw
    DirectX::XMVECTOR delta = DirectX::XMQuaternionRotationAxis(
        DirectX::g_XMIdentityR1, // Y-axis
        yaw
    );

    AddRotation(delta);
}

void RigidBody::AddPitch(float pitch)
{
    DirectX::XMVECTOR delta = DirectX::XMQuaternionRotationAxis(
        DirectX::g_XMIdentityR0, // X-axis
        pitch
    );

    AddRotation(delta);
}

void RigidBody::AddRoll(float roll)
{
    DirectX::XMVECTOR delta = DirectX::XMQuaternionRotationAxis(
        DirectX::g_XMIdentityR2, // Z-axis
        roll
    );

    AddRotation(delta);
}

DirectX::XMFLOAT3 RigidBody::GetRotation() const
{
    return QuaternionToEuler(Orientation);
}

float RigidBody::GetYaw() const
{
    return QuaternionToEuler(Orientation).y;
}

float RigidBody::GetPitch() const
{
    return QuaternionToEuler(Orientation).x;
}

float RigidBody::GetRoll() const
{
    return QuaternionToEuler(Orientation).z;
}

DirectX::XMFLOAT3 RigidBody::QuaternionToEuler(const Quaternion& quaternion)
{
    float r = quaternion.GetR();
    float i = quaternion.GetI();
    float j = quaternion.GetJ();
    float k = quaternion.GetK();

    // Roll (X-axis rotation)
    float sinr_cosp = 2.0f * (r * i + j * k);
    float cosr_cosp = 1.0f - 2.0f * (i * i + j * j);
    float roll = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (Y-axis rotation)
    float sinp = 2.0f * (r * j - k * i);
    float pitch;
    if (std::abs(sinp) >= 1.0f)
        pitch = std::copysign(DirectX::XM_PIDIV2, sinp); // 90 degrees if clamped
    else
        pitch = std::asin(sinp);

    // Yaw (Z-axis rotation)
    float siny_cosp = 2.0f * (r * k + i * j);
    float cosy_cosp = 1.0f - 2.0f * (j * j + k * k);
    float yaw = std::atan2(siny_cosp, cosy_cosp);

    return { pitch, yaw, roll }; // In radians
}

void RigidBody::SetVelocity(const DirectX::XMVECTOR& vel)
{
    Velocity.Set(vel);
    m_VerletNeedsReset = true;
}

void RigidBody::SetDamping(float d)
{
    m_LinearDamping.store(d, std::memory_order_relaxed);
}


void RigidBody::SetElasticity(float e)
{
    m_Elastic.store(e, std::memory_order_relaxed);
}


void RigidBody::SetRestitution(float v)
{
    m_Restitution.store(v, std::memory_order_relaxed);
}


void RigidBody::SetFriction(float v)
{
    m_Friction.store(v, std::memory_order_relaxed);
}


void RigidBody::SetAcceleration(const DirectX::XMVECTOR& acc)
{
    Acceleration.Set(acc);
}


void RigidBody::SetOrientation(const Quaternion& q)
{
    Orientation = q;
    Orientation.Normalize(); // direct math is fine
}


void RigidBody::SetAngularVelocity(const DirectX::XMVECTOR& av)
{
    AngularVelocity.Set(av);
}


void RigidBody::SetMass(float mass)
{
    InverseMass.store((mass > 0.0f) ? 1.0f / mass : 0.0f, std::memory_order_relaxed);
}


void RigidBody::SetInverseMass(float invMass)
{
    InverseMass.store(invMass, std::memory_order_relaxed);
}


void RigidBody::SetLinearDamping(float d)
{
    m_LinearDamping.store(d, std::memory_order_relaxed);
}


void RigidBody::SetAngularDamping(float d)
{
    AngularDamping.store(d, std::memory_order_relaxed);
}


void RigidBody::SetInverseInertiaTensor(const DirectX::XMMATRIX& tensor)
{
    m_InverseInertiaTensorLocal = tensor;
}


// Getters
DirectX::XMVECTOR RigidBody::GetPosition()
{
    return Position.Get();
}

DirectX::XMVECTOR RigidBody::GetVelocity()
{
    return Velocity.Get();
}

DirectX::XMVECTOR RigidBody::GetAcceleration()
{
    return Acceleration.Get();
}

DirectX::XMVECTOR RigidBody::GetAngularVelocity()
{
    return AngularVelocity.Get();
}

Quaternion RigidBody::GetOrientation() const
{
    auto result = Orientation;
    return result;
}

float RigidBody::GetMass() const
{
    float invMass = InverseMass.load(std::memory_order_relaxed);
    return (invMass > 0.0f) ? 1.0f / invMass : INFINITY;
}

float RigidBody::GetElasticity() const
{
    return m_Elastic.load(std::memory_order_relaxed);
}

float RigidBody::GetInverseMass() const
{
    return InverseMass.load(std::memory_order_relaxed);
}

DirectX::XMMATRIX RigidBody::GetInverseInertiaTensor() const
{
    return m_InverseInertiaTensorLocal;
}

DirectX::XMMATRIX RigidBody::GetInverseInertiaTensorWorld() const
{
    return InverseInertiaTensorWorld;
}

bool RigidBody::HasFiniteMass() const
{
    return InverseMass.load(std::memory_order_relaxed) > 0.0f;
}

float RigidBody::GetDamping() const
{
    return m_LinearDamping.load(std::memory_order_relaxed);
}

float RigidBody::GetAngularDamping() const
{
    return AngularDamping.load(std::memory_order_relaxed);
}

float RigidBody::GetRestitution() const
{
    return m_Restitution.load(std::memory_order_relaxed);
}

float RigidBody::GetFriction() const
{
    return m_Friction.load(std::memory_order_relaxed);
}

void RigidBody::SetRestingState(bool state)
{
    m_Resting.store(state, std::memory_order_relaxed);
}

bool RigidBody::GetRestingState() const
{
    return m_Resting.load(std::memory_order_relaxed);
}

void RigidBody::ConstrainVelocity(const DirectX::XMVECTOR& contactNormal)
{
    const DirectX::XMVECTOR velocity = GetVelocity();
    const float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(velocity, contactNormal));

    if (dot < 0.0f)
    {
        DirectX::XMVECTOR correction = DirectX::XMVectorScale(contactNormal, dot);
        DirectX::XMVECTOR constrained = DirectX::XMVectorSubtract(velocity, correction);
        SetVelocity(constrained);
    }
}

void RigidBody::SetAsPlatform(bool state)
{
    m_Platform.store(state, std::memory_order_relaxed);
}

bool RigidBody::IsPlatform() const
{
    return m_Platform.load(std::memory_order_relaxed);
}

void RigidBody::ComputeInverseInertiaTensorBox(float width, float height, float depth)
{
    using namespace DirectX;

    float Ixx = (1.0f / 12.0f) * GetMass() * (height * height + depth * depth);
    float Iyy = (1.0f / 12.0f) * GetMass() * (width * width + depth * depth);
    float Izz = (1.0f / 12.0f) * GetMass() * (width * width + height * height);

    // Invert each component for the inverse tensor
    float invIxx = (Ixx > 0.0f) ? (1.0f / Ixx) : 0.0f;
    float invIyy = (Iyy > 0.0f) ? (1.0f / Iyy) : 0.0f;
    float invIzz = (Izz > 0.0f) ? (1.0f / Izz) : 0.0f;

    m_InverseInertiaTensorLocal = XMMatrixSet(
        invIxx, 0.0f, 0.0f, 0.0f,
        0.0f, invIyy, 0.0f, 0.0f,
        0.0f, 0.0f, invIzz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

void RigidBody::ComputeInverseInertiaTensorSphere(float radius)
{
    float mass = GetMass();
    if (mass <= 0.0f)
    {
        m_InverseInertiaTensorLocal.r[0] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[1] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[2] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[3] = DirectX::XMVectorZero();
        return;
    }

    float factor = 2.0f / 5.0f * mass * radius * radius;
    float inv = 1.0f / factor;

    m_InverseInertiaTensorLocal = DirectX::XMMatrixIdentity();
    m_InverseInertiaTensorLocal.r[0] = DirectX::XMVectorSet(inv, 0, 0, 0);
    m_InverseInertiaTensorLocal.r[1] = DirectX::XMVectorSet(0, inv, 0, 0);
    m_InverseInertiaTensorLocal.r[2] = DirectX::XMVectorSet(0, 0, inv, 0);
    m_InverseInertiaTensorLocal.r[3] = DirectX::XMVectorSet(0, 0, 0, 1);
}

void RigidBody::ComputeInverseInertiaTensorCapsule(float radius, float height)
{
    using namespace DirectX;

    float mass = GetMass();
    if (mass <= 0.0f)
    {
        m_InverseInertiaTensorLocal.r[0] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[1] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[2] = DirectX::XMVectorZero();
        m_InverseInertiaTensorLocal.r[3] = DirectX::XMVectorZero();
        return;
    }

    float r2 = radius * radius;
    float h = height;
    float cylMass = mass * (height / (height + (4.0f / 3.0f) * radius)); // approx mass split
    float sphMass = mass - cylMass;

    // Cylinder inertia (Y-axis is height axis)
    float IxxCyl = 0.25f * cylMass * r2 + (1.0f / 12.0f) * cylMass * h * h;
    float IyyCyl = 0.5f * cylMass * r2;
    float IzzCyl = IxxCyl;

    // Sphere inertia
    float I_sphere = (2.0f / 5.0f) * sphMass * r2;

    // Use parallel axis theorem for spheres
    float offset = (h / 2.0f);
    float IxxSph = I_sphere + sphMass * offset * offset;
    float IyySph = I_sphere;
    float IzzSph = IxxSph;

    // Total inertia
    float Ixx = IxxCyl + IxxSph;
    float Iyy = IyyCyl + IyySph;
    float Izz = IzzCyl + IzzSph;

    // Invert for inverse tensor
    float invIxx = (Ixx > 0.0f) ? 1.0f / Ixx : 0.0f;
    float invIyy = (Iyy > 0.0f) ? 1.0f / Iyy : 0.0f;
    float invIzz = (Izz > 0.0f) ? 1.0f / Izz : 0.0f;

    m_InverseInertiaTensorLocal = XMMatrixSet(
        invIxx, 0.0f, 0.0f, 0.0f,
        0.0f, invIyy, 0.0f, 0.0f,
        0.0f, 0.0f, invIzz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

void RigidBody::ApplyLinearImpulse(const DirectX::XMVECTOR& impulse)
{
    if (GetInverseMass() <= 0.0f)
        return;

    SetVelocity(DirectX::XMVectorAdd(GetVelocity(), DirectX::XMVectorScale(impulse, GetInverseMass())));
}

void RigidBody::ApplyAngularImpulse(const DirectX::XMVECTOR& impulse, const DirectX::XMVECTOR& contactVector)
{
    DirectX::XMVECTOR torque = DirectX::XMVector3Cross(contactVector, impulse);
    DirectX::XMVECTOR deltaAngular = DirectX::XMVector3Transform(torque, InverseInertiaTensorWorld);

    SetAngularVelocity(DirectX::XMVectorAdd(GetAngularVelocity(), deltaAngular));
}

void RigidBody::IntegrateEuler(float dt)
{
    CalculateDerivedData();

    if (InverseMass.load(std::memory_order_relaxed) <= 0.0f) return;

    DirectX::XMVECTOR pos = Position.Get();
    DirectX::XMVECTOR vel = Velocity.Get();
    DirectX::XMVECTOR acc = Acceleration.Get();
    DirectX::XMVECTOR force = ForceAccum.Get();

    DirectX::XMVECTOR acceleration = DirectX::XMVectorAdd(acc, DirectX::XMVectorScale(force, InverseMass));

    pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(vel, dt));
    vel = DirectX::XMVectorAdd(vel, DirectX::XMVectorScale(acceleration, dt));

    ApplyLinearDamping(vel, dt);

    Position.Set(pos);
    Velocity.Set(vel);

    IntegrateAngular(dt);
    ClearAccumulators();
}

void RigidBody::IntegrateSemiImplicitEuler(float dt)
{
    CalculateDerivedData();

    if (InverseMass.load(std::memory_order_relaxed) <= 0.0f) return;

    DirectX::XMVECTOR pos = Position.Get();
    DirectX::XMVECTOR vel = Velocity.Get();
    DirectX::XMVECTOR acc = Acceleration.Get();
    DirectX::XMVECTOR force = ForceAccum.Get();

    DirectX::XMVECTOR acceleration = DirectX::XMVectorAdd(acc, DirectX::XMVectorScale(force, InverseMass));

    vel = DirectX::XMVectorAdd(vel, DirectX::XMVectorScale(acceleration, dt));
    pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(vel, dt));

    ApplyLinearDamping(vel, dt);

    Position.Set(pos);
    Velocity.Set(vel);

    IntegrateAngular(dt);
    ClearAccumulators();
}

void RigidBody::IntegrateVerlet(float dt)
{
    CalculateDerivedData();

    if (InverseMass.load(std::memory_order_relaxed) <= 0.0f) return;

    DirectX::XMVECTOR pos = Position.Get();

    if (m_VerletNeedsReset)
    {
        DirectX::XMVECTOR vel = Velocity.Get();
        DirectX::XMVECTOR lastPos = DirectX::XMVectorSubtract(pos, DirectX::XMVectorScale(vel, dt));
        m_LastPosition.Set(lastPos);
        m_VerletNeedsReset = false;
    }

    DirectX::XMVECTOR lastPos = m_LastPosition.Get();
    DirectX::XMVECTOR acc = Acceleration.Get();
    DirectX::XMVECTOR force = ForceAccum.Get();

    DirectX::XMVECTOR acceleration = DirectX::XMVectorAdd(acc, DirectX::XMVectorScale(force, InverseMass));
    acceleration = ClampVectorLength(acceleration, 100.0f);

    DirectX::XMVECTOR posDelta = DirectX::XMVectorSubtract(pos, lastPos);
    DirectX::XMVECTOR accelTerm = DirectX::XMVectorScale(acceleration, dt * dt);
    DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(pos, DirectX::XMVectorAdd(posDelta, accelTerm));

    DirectX::XMVECTOR vel = DirectX::XMVectorScale(posDelta, 1.0f / dt);
    ApplyLinearDamping(vel, dt);

    m_LastPosition.Set(pos);
    Position.Set(newPos);
    Velocity.Set(vel);

    IntegrateAngular(dt);
    ClearAccumulators();
}

void RigidBody::IntegrateAngular(float dt)
{
    DirectX::XMVECTOR angVel = AngularVelocity.Get();
    DirectX::XMVECTOR torque = TorqueAccum.Get();

    DirectX::XMVECTOR angularAcc = DirectX::XMVector3Transform(torque, m_InverseInertiaTensorLocal);
    angVel = DirectX::XMVectorAdd(angVel, DirectX::XMVectorScale(angularAcc, dt));

    float angDamp = AngularDamping.load(std::memory_order_relaxed);
    angVel = DirectX::XMVectorScale(angVel, std::pow(angDamp, dt));

    AngularVelocity.Set(angVel);

    Orientation.AddScaledVector(angVel, dt);
    Orientation.Normalize();

    // Optional: sleep threshold
    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(angVel)) < 1e-5f)
        AngularVelocity.Set(DirectX::XMVectorZero());
}

void RigidBody::ApplyLinearDamping(DirectX::XMVECTOR& vel, float dt) const
{
    float damping = m_LinearDamping.load(std::memory_order_relaxed);
    vel = DirectX::XMVectorScale(vel, std::pow(damping, dt));

    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(vel)) < 1e-5f)
        vel = DirectX::XMVectorZero();
}

void RigidBody::ResetVerletState(float delta)
{
    const auto pos = Position.Get();
    const auto vel = Velocity.Get();
    const auto lastPos = DirectX::XMVectorSubtract(pos, DirectX::XMVectorScale(vel, delta));
    m_LastPosition.Set(lastPos);
}

DirectX::XMVECTOR RigidBody::ClampVectorLength(DirectX::XMVECTOR vec, float maxLength)
{
    using namespace DirectX;

    XMVECTOR lenSq = XMVector3LengthSq(vec);
    float lengthSquared = XMVectorGetX(lenSq);

    if (lengthSquared > maxLength * maxLength)
    {
        XMVECTOR len = XMVectorSqrt(lenSq);
        vec = XMVectorDivide(XMVectorScale(vec, maxLength), len); // vec * (max / length)
    }

    return vec;
}