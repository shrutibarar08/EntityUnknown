#include "pch.h"
#include "ForceRegistry.h"
#include <algorithm>


void ForceRegistry::Add(ICollider* collider, ForceGenerator* fg)
{
    RegisteredForces.push({ collider, fg });
}

void ForceRegistry::Remove(ICollider* collider, ForceGenerator* fg)
{
    Concurrency::concurrent_queue<ForceRegistration> tempQueue;

    ForceRegistration reg;
    while (RegisteredForces.try_pop(reg))
    {
        if (!(reg.Collider == collider && reg.ForceGenerates == fg))
        {
            tempQueue.push(reg); // Keep it
        }
    }

    // Restore filtered entries
    ForceRegistration requeue;
    while (tempQueue.try_pop(requeue))
    {
        RegisteredForces.push(requeue);
    }
}

void ForceRegistry::Clear()
{
    ForceRegistration reg;
    while (RegisteredForces.try_pop(reg)) { /* drop all */ }
}

void ForceRegistry::UpdateForces(float duration)
{
    Concurrency::concurrent_queue<ForceRegistration> tempQueue;

    ForceRegistration reg;
    while (RegisteredForces.try_pop(reg))
    {
        if (reg.Collider && reg.ForceGenerates)
        {
            reg.ForceGenerates->UpdateForce(reg.Collider, duration);
        }

        // Re-add it after use (preserves queue)
        tempQueue.push(reg);
    }

    // Restore the original queue
    ForceRegistration requeue;
    while (tempQueue.try_pop(requeue))
    {
        RegisteredForces.push(requeue);
    }
}
