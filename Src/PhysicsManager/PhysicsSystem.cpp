#include "PhysicsSystem.h"
#include <ranges>

#include "Utils/Logger/Logger.h"

bool PhysicsSystem::OnInit(const SweetLoader& sweetLoader)
{
	return true;
}

bool PhysicsSystem::OnFrameUpdate(float deltaTime)
{
	Update(deltaTime);
	return true;
}

bool PhysicsSystem::OnFrameEnd()
{
	return ISystem::OnFrameEnd();
}

bool PhysicsSystem::OnExit(SweetLoader& sweetLoader)
{
	return true;
}

std::string PhysicsSystem::GetSystemName()
{
	return "PhysicsSystem";
}

bool PhysicsSystem::AddObject(IRender* renderObj)
{
	ID id = renderObj->GetAssignedID();
	if (m_RenderedObjects.contains(id)) return false;
	m_RenderedObjects[id] = renderObj;
	return true;
}

bool PhysicsSystem::RemoveObject(const IRender* renderObj)
{
	ID id = renderObj->GetAssignedID();

	if (m_RenderedObjects.contains(id)) return RemoveObject(id);
	return false;
}

bool PhysicsSystem::RemoveObject(ID renderObjID)
{
	if (m_RenderedObjects.contains(renderObjID))
	{
		m_RenderedObjects.erase(renderObjID);
		return true;
	}
	return false;
}

void PhysicsSystem::Clear()
{
	m_RenderedObjects.clear();
}

void PhysicsSystem::SetIntegration(IntegrationType type)
{
	m_IntegrationType = type;
}

void PhysicsSystem::Update(float deltaTime)
{
	std::vector<ICollider*> colliders;

	for (auto& obj: m_RenderedObjects | std::views::values)
	{
		ICollider* collider = obj->GetCubeCollider();
		if (!collider) continue;

		//~ Update Collider and cache
		collider->Update(deltaTime);
		colliders.push_back(collider);

		//~ Update Rigid Body
		RigidBody* body = collider->GetRigidBody();
		if (!body) continue;

		body->Integrate(deltaTime, m_IntegrationType);
	}

	// === Collision Detection ===
	std::vector<Contact> contacts;
	for (size_t i = 0; i < colliders.size(); ++i)
	{
		for (size_t j = i + 1; j < colliders.size(); ++j)
		{
			ICollider* colliderA = colliders[i];
			ICollider* colliderB = colliders[j];

			if (!colliderA || !colliderB) continue;

			Contact contact;
			if (colliderA->CheckCollision(colliderB, contact))
			{
				colliderA->RegisterCollision(colliderB);
				contacts.push_back(contact);
			}
		}
	}

	// === Contact Resolution ===
	CollisionResolver::ResolveContacts(contacts, deltaTime);
}
