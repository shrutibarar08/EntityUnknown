#include "LevelEditor.h"

#include "Imgui/imgui.h"
#include "RenderManager/RenderQueue/RenderQueue.h"
#include <ranges>

#include "RenderManager/Model/Cube/ModelCube.h"
#include "RenderManager/RenderQueue/Render3DQueue.h"

bool LevelEditor::OnInit(const SweetLoader& sweetLoader)
{
	return true;
}

bool LevelEditor::OnFrameUpdate(float deltaTime)
{
	return true;
}

bool LevelEditor::OnFrameEnd()
{
	return ISystem::OnFrameEnd();
}

bool LevelEditor::OnExit(SweetLoader& sweetLoader)
{
	return true;
}

std::string LevelEditor::GetSystemName()
{
	return "Level Editor";
}

void LevelEditor::RenderBegin()
{
}

void LevelEditor::RenderExecute()
{
	if (ImGui::BeginMainMenuBar())
	{
		// === File Menu ===
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Rendered Object")) { m_bDisplayRenderObjectUI = !m_bDisplayRenderObjectUI; }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				m_bCreateCubeRenderObjectUI = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	RenderObjectUpdateUI();
	RenderObjectCubeCreationUI();
}

void LevelEditor::RenderEnd()
{
}

void LevelEditor::RenderObjectCubeCreationUI()
{
	if (m_bCreateCubeRenderObjectUI)
	{
		ImGui::OpenPopup(m_RenderPopUpName.c_str());
		m_bCreateCubeRenderObjectUI = false; // Reset the trigger flag so it only fires once
	}

	if (ImGui::BeginPopupModal("Create Render Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Create"))
		{
			auto cube = std::make_unique<ModelCube>();

			// Get camera eye position
			const DirectX::XMFLOAT3 eyePosition = RenderQueueSingleton::Get()->GetCameraController()->GetEyePosition();
			DirectX::XMVECTOR forwardVec = RenderQueueSingleton::Get()->GetCameraController()->GetForwardVector();

			// Convert forward vector to float3
			DirectX::XMFLOAT3 forward;
			DirectX::XMStoreFloat3(&forward, forwardVec);

			// Spawn 5 units in front of camera
			DirectX::XMFLOAT3 spawnPos = {
				eyePosition.x + forward.x * 5.0f,
				eyePosition.y + forward.y * 5.0f,
				eyePosition.z + forward.z * 5.0f
			};

			cube->GetRigidBody()->SetTranslation(spawnPos.x, spawnPos.y, spawnPos.z);

			// Add to render queue
			RenderQueueSingleton::Get()->AddRender(cube.get());
			m_Renders[cube->GetAssignedID()] = std::move(cube);

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void LevelEditor::RenderObjectUpdateUI() const
{
	if (!m_bDisplayRenderObjectUI) return;

	if (!RenderQueueSingleton::IsInitialized()) return;

	ImGui::Begin("Render Object Controls"); // All controls go under this one window

	for (auto& [id, render] : RenderQueueSingleton::Get()->GetRenders())
	{
		if (!RenderQueueSingleton::Get()->IsInside(render)) continue;
		if (!render) continue;

		std::string label = render->GetName() + "##" + std::to_string(id);

		if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::PushID(static_cast<int>(id));
			render->RenderControlUI();
			ImGui::PopID();
		}
	}
	ImGui::End(); // End of main window
}
