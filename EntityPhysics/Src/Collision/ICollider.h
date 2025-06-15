#pragma once

#include <DirectXMath.h>
#include <unordered_map>
#include <functional>

#include "RigidBody/RigidBody.h"


struct Contact;

enum class ColliderType : uint8_t
{
    Cube,
};

enum class ColliderState : uint8_t
{
    Dynamic,
    Static,
    Trigger,
};

class ICollider;

typedef struct TRIGGER_COLLISION_INFO
{
    ICollider* TargetCollider;
    std::function<void()> m_OnTriggerEnterCallbackFn;
    std::function<void()> m_OnTriggerExitCallbackFn;

}TRIGGER_COLLISION_INFO;

class ICollider
{
public:
    ICollider(RigidBody* attachBody);
    virtual ~ICollider() = default;

    ICollider(const ICollider&) = default;
    ICollider(ICollider&&) = default;
    ICollider& operator=(const ICollider&) = default;
    ICollider& operator=(ICollider&&) = default;

    void Update(float deltaTime);

    void SetTriggerTarget(const TRIGGER_COLLISION_INFO& triggerCollisionInfo);

    // Collision interface
    void RegisterCollision(ICollider* other);
    virtual bool CheckCollision(ICollider* other, Contact& outContact)  = 0;
    virtual ColliderType GetColliderType() const                        = 0;
    virtual void SetScale(const DirectX::XMVECTOR& vector)              = 0;
    virtual DirectX::XMVECTOR GetScale() const                          = 0;

    // Getters
    RigidBody* GetRigidBody() const { return m_RigidBody; }
    ColliderState GetColliderState() const;
    const char* ToString() const;
    const char* GetColliderTypeName() const;
    DirectX::XMMATRIX GetTransformationMatrix() const;
    DirectX::XMMATRIX GetWorldMatrix() const;

    // Setters
    void SetColliderState(ColliderState state);

    // For type-safe down casting
    template<typename T>
    T* As();

    template<typename T>
    const T* As() const;

    template<typename T>
    static constexpr const T& Min(const T& a, const T& b);

protected:
    //~ Callback on Collision

    struct ColliderInfo
    {
        bool HasEntered;
        bool HasExited;
        std::function<void()> m_OnTriggerEnterCallbackFn;
        std::function<void()> m_OnTriggerExitCallbackFn;
    };
    std::unordered_map<ICollider*, ColliderInfo> m_CollidersInfo{};

    DirectX::XMMATRIX m_TransformationMatrix{};
    ColliderState m_ColliderState{ ColliderState::Static };
    RigidBody* m_RigidBody;
};

template <typename T>
constexpr const T& ICollider::Min(const T& a, const T& b)
{
    return (a < b) ? a : b;
}

template <typename T>
T* ICollider::As()
{
    return dynamic_cast<T*>(this);
}

template <typename T>
const T* ICollider::As() const
{
    return dynamic_cast<const T*>(this);
}

