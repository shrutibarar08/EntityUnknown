#include "Render2DQueue.h"
#include <ranges>

#include "SystemManager/EventQueue/EventQueue.h"

Render2DQueue::Render2DQueue(CameraController* controller, ID3D11Device* device)
{
	m_Device = device;
	m_CameraController = controller;
	m_Initialized = true;

}

bool Render2DQueue::AddBitmap(IBitmap* bitmap)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_BitmapsToRender.contains(bitmap->GetAssignedID()))
	{
		if (!bitmap->IsInitialized()) bitmap->Build(m_Device);
		m_BitmapsToRender.emplace(bitmap->GetAssignedID(), bitmap);
		status = true;
	}
	return status;
}

bool Render2DQueue::AddBackgroundBitmap(IBitmap* bitmap)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_BitmapsToRenderInBackGround.contains(bitmap->GetAssignedID()))
	{
		if (!bitmap->IsInitialized()) bitmap->Build(m_Device);
		m_BitmapsToRenderInBackGround.emplace(bitmap->GetAssignedID(), bitmap);
		status = true;
	}
	return status;
}

bool Render2DQueue::RemoveBackgroundBitmap(const IBitmap* bitmap)
{
	if (m_BitmapsToRenderInBackGround.empty()) return false;

	bool status = false;
	if (m_BitmapsToRenderInBackGround.contains(bitmap->GetAssignedID()))
	{
		m_BitmapsToRenderInBackGround.erase(bitmap->GetAssignedID());
		status = true;
	}
	return status;
}

bool Render2DQueue::RemoveBackgroundBitmap(uint64_t bitmapId)
{
	if (m_BitmapsToRenderInBackGround.empty()) return false;

	bool status = false;

	if (m_BitmapsToRenderInBackGround.contains(bitmapId))
	{
		m_BitmapsToRenderInBackGround.erase(bitmapId);
		status = true;
	}
	return status;
}

void Render2DQueue::RenderBackgroundAll(ID3D11DeviceContext* context)
{
	if (m_BitmapsToRenderInBackGround.empty()) return;

	for (auto& bitmap : m_BitmapsToRenderInBackGround | std::views::values)
	{
		if (!bitmap->IsInitialized()) continue;
		bitmap->Render(context);
	}
}

bool Render2DQueue::RemoveBitmap(const IBitmap* bitmap)
{
	if (m_BitmapsToRender.empty()) return false;

	bool status = false;
	if (m_BitmapsToRender.contains(bitmap->GetAssignedID()))
	{
		m_BitmapsToRender.erase(bitmap->GetAssignedID());
		status = true;
	}
	return status;
}

bool Render2DQueue::RemoveBitmap(uint64_t bitmapId)
{
	if (m_BitmapsToRender.empty()) return false;

	bool status = false;

	if (m_BitmapsToRender.contains(bitmapId))
	{
		m_BitmapsToRender.erase(bitmapId);
		status = true;
	}
	return status;
}

bool Render2DQueue::UpdateBuffers(ID3D11DeviceContext* context)
{
	CAMERA_INFORMATION_CPU_DESC cb{};
	// Invert them
	cb.ViewMatrix = XMMatrixTranspose(m_CameraController->GetViewMatrix());
	cb.ProjectionMatrix = XMMatrixTranspose(m_CameraController->GetOrthogonalMatrix());
	cb.CameraPosition = m_CameraController->GetEyePosition();

	static int times = 0;
	for (auto& bitmap : m_BitmapsToRender | std::views::values)
	{
		if (!bitmap->IsInitialized()) continue;
		bitmap->SetWorldMatrixData(cb);
	}

	for (auto& bitmap : m_BitmapsToRenderInBackGround | std::views::values)
	{
		if (!bitmap->IsInitialized()) continue;
		bitmap->SetWorldMatrixData(cb);
	}

	return true;
}

void Render2DQueue::RenderAll(ID3D11DeviceContext* context)
{
	if (m_BitmapsToRender.empty()) return;

	for (auto& bitmap : m_BitmapsToRender | std::views::values)
	{
		if (!bitmap->IsInitialized()) continue;
		bitmap->Render(context);
	}
}

void Render2DQueue::Clean()
{
	m_BitmapsToRender.clear();
}
