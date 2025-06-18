#include "Render3DQueue.h"

#include <algorithm>
#include <ranges>

#include "Utils/Logger/Logger.h"


Render3DQueue::Render3DQueue(
	CameraController* controller,
	ID3D11Device* device,
	ID3D11DeviceContext* deviceContext,
	PhysicsSystem* physics)
{
	m_PhysicsSystem = physics;
	m_Device = device;
	m_DeviceContext = deviceContext;
	m_CameraController = controller;
	m_Initialized = true;
}

bool Render3DQueue::AddModel(IModel* model)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_ModelsToRender.contains(model->GetAssignedID()))
	{
		if (!model->IsInitialized())
		{
			LOG_WARNING("BUILDING 3D Model!");
			model->Build(m_Device, m_DeviceContext);
			LOG_SUCCESS("BUILDING 3D Model Complete!");
		}
		m_PhysicsSystem->AddObject(model);
		m_ModelsToRender.emplace(model->GetAssignedID(), model);
		SortModelZBuffer();
		status = true;
	}
	return status;
}

bool Render3DQueue::RemoveModel(const IModel* model)
{
	if (m_ModelsToRender.empty()) return false;

	bool status = false;
	if (m_ModelsToRender.contains(model->GetAssignedID()))
	{
		m_PhysicsSystem->RemoveObject(model);
		m_ModelsToRender.erase(model->GetAssignedID());
		SortModelZBuffer();
		status = true;
	}
	return status;
}

bool Render3DQueue::RemoveModel(uint64_t modelId)
{
	if (m_ModelsToRender.empty()) return false;

	bool status = false;

	if (m_ModelsToRender.contains(modelId))
	{
		m_PhysicsSystem->RemoveObject(modelId);
		m_ModelsToRender.erase(modelId);
		SortModelZBuffer();
		status = true;
	}
	return status;
}

bool Render3DQueue::UpdateVertexConstantBuffer(ID3D11DeviceContext* context)
{
	if (m_ModelsToRender.empty()) return false;

	CAMERA_INFORMATION_CPU_DESC cb{};
	// Invert them
	cb.ViewMatrix = XMMatrixTranspose(m_CameraController->GetViewMatrix());
	cb.ProjectionMatrix = XMMatrixTranspose(m_CameraController->GetProjectionMatrix());
	cb.CameraPosition = m_CameraController->GetEyePosition();

	static int times = 0;
	for (auto& model : m_ModelsToRender | std::views::values)
	{
		if (!model->IsInitialized()) continue;

		model->SetWorldMatrixData(cb);

		for (auto& light : m_LightsToRender | std::views::values)
		{
			model->AddLight(light);
		}
	}
	return true;
}

void Render3DQueue::RenderAll(ID3D11DeviceContext* context)
{
	if (m_ModelsToRender.empty()) return;


	for (auto& model : m_ModelsToRender | std::views::values)
	{
		if (!model->IsInitialized()) continue;
		model->Render(context);
	}
}

void Render3DQueue::Clean()
{
	m_ModelsToRender.clear();
}

bool Render3DQueue::AddLight(ILightSource* light)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_LightsToRender.contains(light->GetAssignedID()))
	{
		m_LightsToRender.emplace(light->GetAssignedID(), light);
		status = true;
	}
	return status;
}

bool Render3DQueue::RemoveLight(ILightSource* light)
{
	if (m_LightsToRender.empty()) return false;

	bool status = false;
	if (m_LightsToRender.contains(light->GetAssignedID()))
	{
		for (auto& object : m_ModelsToRender | std::views::values)
		{
			object->RemoveLight(light);
		}
		m_LightsToRender.erase(light->GetAssignedID());
		status = true;
	}
	return status;
}

void Render3DQueue::SortModelZBuffer()
{
	m_SortedModels.clear();
	m_SortedModels.reserve(m_ModelsToRender.size());

	for (auto& model : m_ModelsToRender | std::views::values)
	{
		m_SortedModels.push_back(model);
	}

	std::sort(m_SortedModels.begin(), m_SortedModels.end(),
	[](IModel* a, IModel* b)
	{
		return a->GetRigidBody()->GetPositionZ() > b->GetRigidBody()->GetPositionZ(); // back-to-front
	});
}
