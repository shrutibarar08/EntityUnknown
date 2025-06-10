#include "Render3DQueue.h"

#include "Utils/Logger/Logger.h"

#include <ranges>

Render3DQueue::Render3DQueue(CameraController* controller, ID3D11Device* device)
{
	m_Device = device;
	m_CameraController = controller;
	m_Initialized = true;
}

bool Render3DQueue::AddModel(IModel* model)
{
	if (!m_Initialized) return false;
	bool status = false;
	if (!m_ModelsToRender.contains(model->GetAssignedID()))
	{
		if (!model->IsInitialized()) model->Build(m_Device);
		m_ModelsToRender.emplace(model->GetAssignedID(), model);
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
		m_ModelsToRender.erase(model->GetAssignedID());
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
		m_ModelsToRender.erase(modelId);
		status = true;
	}
	return status;
}

bool Render3DQueue::UpdateVertexConstantBuffer(ID3D11DeviceContext* context)
{
	if (m_ModelsToRender.empty()) return false;

	CAMERA_MATRIX_DESC cb{};
	// Invert them
	cb.ViewMatrix = XMMatrixTranspose(m_CameraController->GetViewMatrix());
	cb.ProjectionMatrix = XMMatrixTranspose(m_CameraController->GetProjectionMatrix());

	static int times = 0;
	for (auto& model : m_ModelsToRender | std::views::values)
	{
		if (!model->IsInitialized()) continue;

		model->UpdateTransformation(&cb);
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
