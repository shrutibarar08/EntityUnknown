#include "LevelEditor.h"

#include "Imgui/imgui.h"
#include "RenderManager/RenderQueue/RenderQueue.h"
#include <ranges>

#include "RenderManager/Model/Cube/ModelCube.h"
#include "RenderManager/Model/Mesh/Mesh.h"
#include "RenderManager/RenderQueue/Render3DQueue.h"

bool LevelEditor::OnInit(const SweetLoader& sweetLoader)
{
	return true;
}

bool LevelEditor::OnFrameUpdate(float deltaTime)
{
	return true;
}

bool LevelEditor::OnFrameClear()
{
	return ISystem::OnFrameClear();
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
	RenderMenuUI();
}

void LevelEditor::RenderEnd()
{
}

void LevelEditor::RenderMenuUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Rendered Object")) m_bDisplayRenderObjectUI = !m_bDisplayRenderObjectUI;
			if (ImGui::MenuItem("Background Sprites")) m_bDisplayBackgroundObjectUI = !m_bDisplayBackgroundObjectUI;
			if (ImGui::MenuItem("Front Sprites")) m_bDisplayFrontObjectUI = !m_bDisplayFrontObjectUI;
			if (ImGui::MenuItem("Space Sprites")) m_bDisplaySpaceObjectUI = !m_bDisplaySpaceObjectUI;
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Cube")) m_bCreateCubeRenderObjectUI = true;
			if (ImGui::MenuItem("OBJ")) m_bCreateOBJRenderObjectUI = true;
			if (ImGui::MenuItem("Background Sprite")) m_bCreateBackgroundRenderObjectUI = true;
			if (ImGui::MenuItem("Front Sprite")) m_bCreateFrontRenderObjectUI = true;
			if (ImGui::MenuItem("Space Sprite")) m_bCreateSpaceRenderObjectUI = true;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	Render3DObjectControlsUI();
	RenderObjectCubeCreationUI();
	RenderOBJCreationUI();

	//~ Background Sprites
	RenderBackgroundSpriteCreationUI();
	RenderBackgroundSpriteControlUI();

	//~ Front Sprites
	RenderFrontSpriteControlUI();
	RenderFrontSpriteCreationUI();

	//~ Space Sprite
	RenderSpaceSpriteControlUI();
	RenderSpaceSpriteCreationUI();
}

void LevelEditor::RenderObjectCubeCreationUI()
{
	if (m_bCreateCubeRenderObjectUI)
	{
		ImGui::OpenPopup("Create Render Object");
		m_bCreateCubeRenderObjectUI = false;
	}

	if (ImGui::BeginPopupModal("Create Render Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		auto cube = std::make_unique<ModelCube>();

		// Get camera eye position
		const DirectX::XMFLOAT3 eyePosition = RenderQueueSingleton::Get()->GetCameraController()->GetEyePosition();
		DirectX::XMVECTOR forwardVec = RenderQueueSingleton::Get()->GetCameraController()->GetForwardVector();

		// Convert forward vector to float3
		DirectX::XMFLOAT3 forward;
		DirectX::XMStoreFloat3(&forward, forwardVec);

		// Spawn 5 units in front of camera
		DirectX::XMFLOAT3 spawnPos = 
		{
			eyePosition.x + forward.x * 5.0f,
			eyePosition.y + forward.y * 5.0f,
			eyePosition.z + forward.z * 5.0f
		};

		cube->GetRigidBody()->SetTranslation(spawnPos.x, spawnPos.y, spawnPos.z);

		// Add to render queue
		RenderQueueSingleton::Get()->AddRender(cube.get());
		m_Renders[cube->GetAssignedID()] = std::move(cube);

		ImGui::SameLine();
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void LevelEditor::Render3DObjectControlsUI() const
{
	if (!m_bDisplayRenderObjectUI) return;

	if (!RenderQueueSingleton::IsInitialized()) return;

	ImGui::Begin("Render Object Controls"); // All controls go under this one window

	for (auto& [id, render] : m_Renders)
	{
		if (!RenderQueueSingleton::Get()->IsInside(render.get())) continue;
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

void LevelEditor::RenderOBJCreationUI()
{
	if (m_bCreateOBJRenderObjectUI)
	{
		ImGui::OpenPopup(m_RenderOBJPopUpName.c_str());
		m_bCreateOBJRenderObjectUI = false;
	}

	if (ImGui::BeginPopupModal(m_RenderOBJPopUpName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (m_HolderMesh == nullptr)
		{
			m_HolderMesh = std::make_unique<Mesh>();
		}
		m_HolderMesh->RenderControlUI();

		if (ImGui::Button("Create"))
		{
			m_HolderMesh->Build(RenderQueueSingleton::Get()->m_Device, RenderQueueSingleton::Get()->m_DeviceContext);

			if (m_HolderMesh->IsInitialized())
			{
				// Get camera eye position
				const DirectX::XMFLOAT3 eyePosition = RenderQueueSingleton::Get()->GetCameraController()->GetEyePosition();
				DirectX::XMVECTOR forwardVec = RenderQueueSingleton::Get()->GetCameraController()->GetForwardVector();

				// Convert forward vector to float3
				DirectX::XMFLOAT3 forward;
				DirectX::XMStoreFloat3(&forward, forwardVec);

				// Spawn 5 units in front of camera
				DirectX::XMFLOAT3 spawnPos =
				{
					eyePosition.x + forward.x * 5.0f,
					eyePosition.y + forward.y * 5.0f,
					eyePosition.z + forward.z * 5.0f
				};

				m_HolderMesh->GetRigidBody()->SetTranslation(spawnPos.x, spawnPos.y, spawnPos.z);

				RenderQueueSingleton::Get()->AddRender(m_HolderMesh.get());
				m_Renders[m_HolderMesh->GetAssignedID()] = std::move(m_HolderMesh);
				m_HolderMesh = nullptr;
			}
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

void LevelEditor::RenderBackgroundSpriteControlUI() const
{
	if (!m_bDisplayBackgroundObjectUI) return;

	if (!RenderQueueSingleton::IsInitialized()) return;

	ImGui::Begin("Background Sprite Controls"); // All controls go under this one window

	for (auto& [id, render] : m_BackgroundSprites)
	{
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

void LevelEditor::RenderBackgroundSpriteCreationUI()
{
	if (m_bCreateBackgroundRenderObjectUI)
	{
		ImGui::OpenPopup(m_RenderBackgroundPopUpName.c_str());
		m_bCreateBackgroundRenderObjectUI = false;
	}

	if (ImGui::BeginPopupModal(m_RenderBackgroundPopUpName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (m_BackgroundHolderSprite == nullptr)
		{
			m_BackgroundHolderSprite = std::make_unique<BackgroundSprite>();
		}
		m_BackgroundHolderSprite->RenderControlUI();

		if (ImGui::Button("Create"))
		{
			m_BackgroundHolderSprite->Build(RenderQueueSingleton::Get()->m_Device, RenderQueueSingleton::Get()->m_DeviceContext);
			if (m_BackgroundHolderSprite->IsInitialized())
			{
				RenderQueueSingleton::Get()->AddRenderBackground(m_BackgroundHolderSprite.get());
				m_BackgroundSprites[m_BackgroundHolderSprite->GetAssignedID()] = std::move(m_BackgroundHolderSprite);
				m_BackgroundHolderSprite = nullptr;
			}
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

void LevelEditor::RenderFrontSpriteControlUI() const
{
	if (!m_bDisplayFrontObjectUI) return;

	if (!RenderQueueSingleton::IsInitialized()) return;

	ImGui::Begin("Front Sprite Controls"); // All controls go under this one window

	for (auto& [id, render] : m_FrontSprites)
	{
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

void LevelEditor::RenderFrontSpriteCreationUI()
{
	if (m_bCreateFrontRenderObjectUI)
	{
		ImGui::OpenPopup(m_RenderFrontPopUpName.c_str());
		m_bCreateFrontRenderObjectUI = false;
	}

	if (ImGui::BeginPopupModal(m_RenderFrontPopUpName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (m_FrontHolderSprite == nullptr)
		{
			m_FrontHolderSprite = std::make_unique<ScreenSprite>();
		}
		m_FrontHolderSprite->RenderControlUI();

		if (ImGui::Button("Create"))
		{
			m_FrontHolderSprite->Build(RenderQueueSingleton::Get()->m_Device, RenderQueueSingleton::Get()->m_DeviceContext);
			if (m_FrontHolderSprite->IsInitialized())
			{
				RenderQueueSingleton::Get()->AddRenderFront(m_FrontHolderSprite.get());
				m_FrontSprites[m_FrontHolderSprite->GetAssignedID()] = std::move(m_FrontHolderSprite);
				m_FrontHolderSprite = nullptr;
			}
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

void LevelEditor::RenderSpaceSpriteControlUI() const
{
	if (!m_bDisplaySpaceObjectUI) return;

	if (!RenderQueueSingleton::IsInitialized()) return;

	ImGui::Begin("World Space Sprite Controls"); // All controls go under this one window

	for (auto& [id, render] : m_SpaceSprites)
	{
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

void LevelEditor::RenderSpaceSpriteCreationUI()
{
	if (m_bCreateSpaceRenderObjectUI)
	{
		ImGui::OpenPopup(m_RenderSpacePopUpName.c_str());
		m_bCreateSpaceRenderObjectUI = false;
	}

	if (ImGui::BeginPopupModal(m_RenderSpacePopUpName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (m_SpaceHolderSprite == nullptr)
		{
			m_SpaceHolderSprite = std::make_unique<WorldSpaceSprite>();
		}
		m_SpaceHolderSprite->RenderControlUI();

		if (ImGui::Button("Create"))
		{
			m_SpaceHolderSprite->Build(RenderQueueSingleton::Get()->m_Device, RenderQueueSingleton::Get()->m_DeviceContext);
			if (m_SpaceHolderSprite->IsInitialized())
			{
				RenderQueueSingleton::Get()->AddRender(m_SpaceHolderSprite.get());
				m_SpaceSprites[m_SpaceHolderSprite->GetAssignedID()] = std::move(m_SpaceHolderSprite);
				m_SpaceHolderSprite = nullptr;
			}
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
