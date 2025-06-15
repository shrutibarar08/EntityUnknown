#pragma once
#include <atomic>
#include <DirectXMath.h>
#include "Quaternion/Quaternion.h"
#include "IntegrationType.h"


struct AtomicVector
{
    DirectX::XMVECTOR value = DirectX::XMVectorZero();
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    DirectX::XMVECTOR Get()
    {
        while (lock.test_and_set(std::memory_order_acquire)); // spin
        DirectX::XMVECTOR val = value;
        lock.clear(std::memory_order_release);
        return val;
    }

    void Set(DirectX::XMVECTOR v)
    {
        while (lock.test_and_set(std::memory_order_acquire)); // spin
        value = v;
        lock.clear(std::memory_order_release);
    }
};


class RigidBody
{
public:
    RigidBody();

    void CalculateDerivedData();

    void ClearAccumulators();
    void AddForce(const DirectX::XMVECTOR& force);
    void AddTorque(const DirectX::XMVECTOR& torque);
    void Integrate(float dt, IntegrationType type = IntegrationType::SemiImplicitEuler);
    DirectX::XMMATRIX GetTransformMatrix();

    // Setters
    void SetVelocity(const DirectX::XMVECTOR& vel);
    void SetAcceleration(const DirectX::XMVECTOR& acc);
    void SetAngularVelocity(const DirectX::XMVECTOR& av);
    void SetMass(float mass);
    void SetInverseMass(float invMass);
    void SetLinearDamping(float d);
    void SetAngularDamping(float d);
    void SetInverseInertiaTensor(const DirectX::XMMATRIX& tensor);
    void SetDamping(float d);
    void SetElasticity(float e);
    void SetRestitution(float v);
    void SetFriction(float v);

    // Getters
    DirectX::XMVECTOR GetVelocity();
    DirectX::XMVECTOR GetAcceleration();
    DirectX::XMVECTOR GetAngularVelocity();
    float GetMass() const;
    float GetElasticity() const;
    float GetInverseMass() const;
    DirectX::XMMATRIX GetInverseInertiaTensor() const;
    DirectX::XMMATRIX GetInverseInertiaTensorWorld() const;
    bool HasFiniteMass() const;
    float GetDamping() const;
    float GetAngularDamping() const;
    float GetRestitution() const;
    float GetFriction() const;

    void SetRestingState(bool state);
    bool GetRestingState() const;

    void ConstrainVelocity(const DirectX::XMVECTOR& contactNormal);

    void SetAsPlatform(bool state);
    bool IsPlatform() const;

    void ComputeInverseInertiaTensorBox(float width, float height, float depth);
    void ComputeInverseInertiaTensorSphere(float radius);
    void ComputeInverseInertiaTensorCapsule(float radius, float height);

    void ApplyLinearImpulse(const DirectX::XMVECTOR& impulse);
    void ApplyAngularImpulse(const DirectX::XMVECTOR& impulse, const DirectX::XMVECTOR& contactVector);

    // --- Translation ---
    void SetPosition(const DirectX::XMVECTOR& pos);
    void SetTranslation(float x, float y, float z);
    void SetTranslation(const DirectX::XMFLOAT3& pos);
    void SetTranslation(const DirectX::XMVECTOR& pos);
    void SetTranslation(float x, float y);
    void SetTranslationXY(const DirectX::XMFLOAT2& pos);
    void SetTranslationX(float x);
    void SetTranslationY(float y);
    void SetTranslationZ(float z);

    void AddTranslation(float x, float y, float z);
    void AddTranslation(const DirectX::XMFLOAT3& pos);
    void AddTranslation(const DirectX::XMVECTOR& pos);
    void AddTranslation(float x, float y);
    void AddTranslationXY(const DirectX::XMFLOAT2& pos);
    void AddTranslationX(float x);
    void AddTranslationY(float y);
    void AddTranslationZ(float z);

    DirectX::XMFLOAT3 GetTranslation() ;
    DirectX::XMFLOAT2 GetTranslationXY();
    DirectX::XMVECTOR GetPosition();
    float GetPositionX();
    float GetPositionY();
    float GetPositionZ();

    // ---- Rotation (in radians) ----
    void SetOrientation(const Quaternion& q);
    void SetRotation(float pitch, float yaw, float roll); // standard: pitch=X, yaw=Y, roll=Z
    void SetRotation(const DirectX::XMFLOAT3& rot);
    void SetRotation(const DirectX::XMVECTOR& rot);
    void SetYaw(float yaw);   // Y-axis
    void SetPitch(float pitch); // X-axis
    void SetRoll(float roll); // Z-axis

    void AddRotation(float pitch, float yaw, float roll);
    void AddRotation(const DirectX::XMFLOAT3& rot);
    void AddRotation(const DirectX::XMVECTOR& rot);
    void AddYaw(float yaw);
    void AddPitch(float pitch);
    void AddRoll(float roll);

    Quaternion GetOrientation() const;
    DirectX::XMFLOAT3 GetRotation() const;
    float GetYaw() const;
    float GetPitch() const;
    float GetRoll() const;

    //~ Helper
    static DirectX::XMFLOAT3 QuaternionToEuler(const Quaternion& quaternion);
    static DirectX::XMVECTOR ClampVectorLength(DirectX::XMVECTOR vec, float maxLength);

private:
    void IntegrateEuler(float dt);
    void IntegrateSemiImplicitEuler(float dt);
    void IntegrateVerlet(float dt);

    void IntegrateAngular(float dt);
    void ApplyLinearDamping(DirectX::XMVECTOR& vel, float dt) const;

    void ResetVerletState(float delta);

private:
    bool m_VerletNeedsReset{ false };
    std::atomic<bool> m_Platform{ false };
    std::atomic<bool> m_Resting{ false };
    std::atomic<float> InverseMass{ 10.f };
    std::atomic<float> m_LinearDamping{ 0.75f };
    std::atomic<float> m_Elastic{ 0.56f };
    std::atomic<float> m_Restitution{ 0.35f };
    std::atomic<float> m_Friction{ 0.38f };
    std::atomic<float> AngularDamping{ 0.39f };


    Quaternion Orientation;
    AtomicVector Position;
    AtomicVector m_LastPosition;
    AtomicVector Velocity;
    AtomicVector Acceleration;
    AtomicVector ForceAccum;
    AtomicVector AngularVelocity;
    AtomicVector TorqueAccum;

    DirectX::XMMATRIX m_InverseInertiaTensorLocal;
    DirectX::XMMATRIX InverseInertiaTensorWorld;
    DirectX::XMMATRIX TransformMatrix;
};