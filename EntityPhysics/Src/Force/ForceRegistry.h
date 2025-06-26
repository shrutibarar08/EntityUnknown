#pragma once
#include <vector>
#include <concurrent_queue.h>

#include "Collision/ICollider.h"
#include "ForceGenerator.h"


class ForceRegistry
{
public:
    void Add(ICollider* collider, ForceGenerator* fg);
    void Remove(ICollider* collider, ForceGenerator* fg);
    void Clear();
    void UpdateForces(float duration);

protected:
    struct ForceRegistration
    {
        ICollider* Collider;
        ForceGenerator* ForceGenerates;
    };

    Concurrency::concurrent_queue<ForceRegistration> RegisteredForces;
};
