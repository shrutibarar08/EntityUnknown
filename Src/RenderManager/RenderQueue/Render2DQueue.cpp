#include "Render2DQueue.h"
#include <ranges>

#include "SystemManager/EventQueue/EventQueue.h"

Render2DQueue::Render2DQueue(CameraController* controller, ID3D11Device* device, PhysicsSystem* physics)
{
	m_PhysicsSystem = physics;
	m_Device = device;
	m_CameraController = controller;
	m_Initialized = true;
}

bool Render2DQueue::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
	UpdateScreenSprite(deviceContext);
	UpdateBackgroundSprite(deviceContext);
	UpdateSpaceSprite(deviceContext);
	return true;
}

void Render2DQueue::UpdateScreenSize(int width, int height)
{
	m_ScreenHeight = height;
	m_ScreenWidth = width;
}

void Render2DQueue::Clean()
{
	m_ScreenSprites.clear();
}

bool Render2DQueue::AddBackgroundSprite(BackgroundSprite* sprite)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_BackgroundSprites.contains(sprite->GetAssignedID()))
	{
		if (!sprite->IsInitialized()) sprite->Build(m_Device);
		m_BackgroundSprites.emplace(sprite->GetAssignedID(), sprite);
		status = true;
	}
	return status;
}

bool Render2DQueue::RemoveBackgroundSprite(const BackgroundSprite* sprite)
{
	if (m_BackgroundSprites.empty()) return false;
	return RemoveBackgroundSprite(sprite->GetAssignedID());
}

bool Render2DQueue::RemoveBackgroundSprite(uint64_t spriteID)
{
	if (m_BackgroundSprites.empty()) return false;

	if (m_BackgroundSprites.contains(spriteID))
	{
		m_BackgroundSprites.erase(spriteID);
		return true;
	}
	return false;
}

bool Render2DQueue::UpdateBackgroundSprite(ID3D11DeviceContext* deviceContext)
{
	CAMERA_INFORMATION_CPU_DESC cb{};
	// Invert them
	cb.ViewMatrix = XMMatrixTranspose(m_CameraController->GetViewMatrix());
	cb.ProjectionMatrix = XMMatrixTranspose(m_CameraController->GetOrthogonalMatrix());
	cb.CameraPosition = m_CameraController->GetEyePosition();

	for (auto& sprite : m_BackgroundSprites | std::views::values)
	{
		if (!sprite->IsInitialized()) continue;
		sprite->SetWorldMatrixData(cb);
		sprite->SetScreenHeight(m_ScreenHeight);
		sprite->SetScreenWidth(m_ScreenWidth);

		for (auto& light : m_Lights | std::views::values) sprite->AddLight(light);
		
	}
	return true;
}

void Render2DQueue::RenderBackgroundSprites(ID3D11DeviceContext* deviceContext)
{
	if (m_BackgroundSprites.empty()) return;

	for (auto& bitmap : m_BackgroundSprites | std::views::values)
	{
		if (!bitmap->IsInitialized()) continue;
		bitmap->Render(deviceContext);
	}
}

bool Render2DQueue::AddScreenSprite(ScreenSprite* sprite)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_ScreenSprites.contains(sprite->GetAssignedID()))
	{
		if (!sprite->IsInitialized()) sprite->Build(m_Device);
		m_ScreenSprites.emplace(sprite->GetAssignedID(), sprite);
		status = true;
	}
	return status;
}

bool Render2DQueue::RemoveScreenSprite(const ScreenSprite* sprite)
{
	if (m_ScreenSprites.empty()) return false;
	return RemoveScreenSprite(sprite->GetAssignedID());
}

bool Render2DQueue::RemoveScreenSprite(uint64_t spriteID)
{
	if (m_ScreenSprites.empty()) return false;

	bool status = false;

	if (m_ScreenSprites.contains(spriteID))
	{
		m_ScreenSprites.erase(spriteID);
		status = true;
	}
	return status;
}

bool Render2DQueue::UpdateScreenSprite(ID3D11DeviceContext* deviceContext)
{
	CAMERA_INFORMATION_CPU_DESC cb{};
	cb.ViewMatrix = XMMatrixTranspose(m_CameraController->GetViewMatrix());
	cb.ProjectionMatrix = XMMatrixTranspose(m_CameraController->GetOrthogonalMatrix());
	cb.CameraPosition = m_CameraController->GetEyePosition();

	for (auto& sprite : m_ScreenSprites | std::views::values)
	{
		if (!sprite->IsInitialized()) continue;
		sprite->SetWorldMatrixData(cb);
		sprite->SetScreenHeight(m_ScreenHeight);
		sprite->SetScreenWidth(m_ScreenWidth);

		for (auto& light : m_Lights | std::views::values)
		{
			sprite->AddLight(light);
		}
	}
	return true;
}

void Render2DQueue::RenderScreenSprites(ID3D11DeviceContext* deviceContext)
{
	if (m_ScreenSprites.empty()) return;

	for (auto& sprite : m_ScreenSprites | std::views::values)
	{
		if (!sprite->IsInitialized()) continue;
		sprite->Render(deviceContext);
	}
}

bool Render2DQueue::AddSpaceSprite(WorldSpaceSprite* sprite)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_WorldSpaceSprites.contains(sprite->GetAssignedID()))
	{
		if (!sprite->IsInitialized()) sprite->Build(m_Device);
		m_WorldSpaceSprites.emplace(sprite->GetAssignedID(), sprite);
		m_PhysicsSystem->AddObject(sprite);
		SortWorldSpaceSprites();
		status = true;
	}
	return status;
}

bool Render2DQueue::RemoveSpaceSprite(const WorldSpaceSprite* sprite)
{
	if (m_WorldSpaceSprites.empty()) return false;

	ID id = sprite->GetAssignedID();
	return RemoveSpaceSprite(id);
}

bool Render2DQueue::RemoveSpaceSprite(uint64_t spriteId)
{
	if (m_WorldSpaceSprites.empty()) return false;

	if (m_WorldSpaceSprites.contains(spriteId))
	{
		m_WorldSpaceSprites.erase(spriteId);
		m_PhysicsSystem->RemoveObject(spriteId);
		SortWorldSpaceSprites();
		return true;
	}
	return false;
}

bool Render2DQueue::UpdateSpaceSprite(ID3D11DeviceContext* deviceContext)
{
	CAMERA_INFORMATION_CPU_DESC cb{};

	cb.ViewMatrix = XMMatrixTranspose(m_CameraController->GetViewMatrix());
	cb.ProjectionMatrix = XMMatrixTranspose(m_CameraController->GetProjectionMatrix());
	cb.CameraPosition = m_CameraController->GetEyePosition();

	for (auto& sprite : m_WorldSpaceSprites | std::views::values)
	{
		if (!sprite->IsInitialized()) continue;
		sprite->SetWorldMatrixData(cb);

		for (auto& light : m_Lights | std::views::values)
		{
			sprite->AddLight(light);
		}
	}
	return true;
}

void Render2DQueue::RenderSpaceSprites(ID3D11DeviceContext* deviceContext)
{
	if (m_WorldSpaceSprites.empty()) return;

	for (auto& sprite : m_SortedSpaceSprites)
	{
		if (!sprite->IsInitialized()) continue;
		sprite->Render(deviceContext);
	}
}

bool Render2DQueue::AddLight(ILightSource* light)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_Lights.contains(light->GetAssignedID()))
	{
		m_Lights.emplace(light->GetAssignedID(), light);
		status = true;
	}
	return status;
}

bool Render2DQueue::RemoveLight(ILightSource* light)
{
	if (m_Lights.empty()) return false;

	bool status = false;
	if (m_Lights.contains(light->GetAssignedID()))
	{
		for (auto& object : m_BackgroundSprites | std::views::values)
		{
			object->RemoveLight(light);
		}
		m_Lights.erase(light->GetAssignedID());
		status = true;
	}
	return status;
}

void Render2DQueue::SortBackgroundSprites()
{
	m_SortedBackgroundSprites.clear();
	m_SortedBackgroundSprites.reserve(m_BackgroundSprites.size());

	for (auto& sprite : m_BackgroundSprites | std::views::values)
	{
		m_SortedBackgroundSprites.push_back(sprite);
	}

	std::sort(m_SortedBackgroundSprites.begin(), m_SortedBackgroundSprites.end(),
		[](BackgroundSprite* a, BackgroundSprite* b)
		{
			return a->GetRigidBody()->GetPositionZ() > b->GetRigidBody()->GetPositionZ(); // back-to-front
		});
}

void Render2DQueue::SortScreenSprites()
{
	m_SortedScreenSprites.clear();
	m_SortedScreenSprites.reserve(m_WorldSpaceSprites.size());

	for (auto& sprite : m_ScreenSprites | std::views::values)
	{
		m_SortedScreenSprites.push_back(sprite);
	}

	std::sort(m_SortedScreenSprites.begin(), m_SortedScreenSprites.end(),
		[](ScreenSprite* a, ScreenSprite* b)
		{
			return a->GetRigidBody()->GetPositionZ() > b->GetRigidBody()->GetPositionZ(); // back-to-front
		});
}

void Render2DQueue::SortWorldSpaceSprites()
{
	m_SortedSpaceSprites.clear();
	m_SortedSpaceSprites.reserve(m_WorldSpaceSprites.size());

	for (auto& sprite : m_WorldSpaceSprites | std::views::values)
	{
		m_SortedSpaceSprites.push_back(sprite);
	}

	std::sort(m_SortedSpaceSprites.begin(), m_SortedSpaceSprites.end(),
		[](WorldSpaceSprite* a, WorldSpaceSprite* b)
		{
			return a->GetRigidBody()->GetPositionZ() > b->GetRigidBody()->GetPositionZ(); // back-to-front
		});
}
