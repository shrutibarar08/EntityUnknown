#pragma once
#include <vector>
#include "Collision/Contact.h"

class CollisionResolver
{
public:
    // Resolve a single contact (positional + velocity)
    static void ResolveContact(Contact& contact, float deltaTime);

    // Resolve a batch of contacts (e.g. from PhysicsManager)
    static void ResolveContacts(std::vector<Contact>& contacts, float deltaTime);

private:
    //~ Cube vs Cube
    static void ResolveContactWithCubeVsCube(Contact& contact, float deltaTime);
    static void ResolvePenetration(Contact& contact, float deltaTime);
    static void ResolveVelocityWithCubeVsCube(Contact& contact, float deltaTime);
    static void ResolveFrictionWithCubeVsCube(Contact& contact, float deltaTime);
    static void ResolveAngularDampingWithCubeVsCube(Contact& contact, float deltaTime);

    //~ Helper Functions
    static bool IsStatic(const ICollider* collider);
};
