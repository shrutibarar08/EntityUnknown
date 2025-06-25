#include "LevelEditor.h"

#include "Imgui/imgui.h"
#include "RenderManager/RenderQueue/RenderQueue.h"
#include <ranges>

#include "RenderManager/Model/Cube/ModelCube.h"
#include "RenderManager/Model/Mesh/Mesh.h"
#include "RenderManager/RenderQueue/Render3DQueue.h"
#include "SystemManager/Registry/RegistryLight.h"

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

void LevelEditor::AttachRenderToEdit(IRender* render)
{
	if (render)
	{
		m_AttachedToEdit[render->GetAssignedID()] = render;

		if (RenderQueueSingleton::IsInitialized())
		{
			RenderQueueSingleton::Get()->AddRender(render);
		}
	}
}

SweetLoader LevelEditor::GetLevelConfig() const
{
	return m_LevelEditorConfig;
}

void LevelEditor::LoadLevel(const SweetLoader& sweetLevelData)
{
	// Load LevelDataPath or fallback
	std::string path = sweetLevelData["LevelDataPath"].GetValue();
	if (path.empty()) path = DEFAULT_LEVEL_DATA_PATH;

	m_LevelData.Load(path);
	m_LevelData.GetOrCreate("LevelDataPath") = path;

	LoadObjects();
	LoadLights();
	//LoadCameraConfig();
}

void LevelEditor::SaveSweetData(SweetLoader& data)
{
	//~ Save Path Info
	std::string path = m_LevelData["LevelDataPath"].GetValue();
	if (path.empty()) path = DEFAULT_LEVEL_DATA_PATH;

	m_LevelData.Clear();
	data.GetOrCreate("LevelDataPath") = path;

	SaveObjects();
	SaveLights();
	//SaveCameraConfig();
	m_LevelData.Save(path);
}

void LevelEditor::LoadObjects()
{
	// Create objects from loaded data
	for (auto& key : m_LevelData["ObjectData"])
	{
		const SweetLoader& objectData = key.second;
		std::string type = objectData["Type"].GetValue();
		if (type.empty()) continue;

		std::unique_ptr<IRender> model = RegistryMesh::Create(type);
		if (!model) continue;

		model->SetSweetData(objectData);

		if (type == "ModelCube" || type == "Mesh" || type == "WorldSpaceSprite")
		{
			if (RenderQueueSingleton::IsInitialized())
				RenderQueueSingleton::Get()->AddRender(model.get());

			m_Renders[model->GetAssignedID()] = std::move(model);
		}
		else if (type == "BackgroundSprite")
		{
			if (RenderQueueSingleton::IsInitialized())
				RenderQueueSingleton::Get()->AddRenderBackground(model.get());

			BackgroundSprite* rawPtr = dynamic_cast<BackgroundSprite*>(model.get());
			if (rawPtr)
			{
				m_BackgroundSprites[rawPtr->GetAssignedID()] = std::unique_ptr<BackgroundSprite>(rawPtr);
				model.release(); // prevent double-delete
			}
			else
			{
				LOG_ERROR("Failed to cast IRender to BackgroundSprite for object: " + key.first);
			}
		}
		else if (type == "ScreenSprite")
		{
			if (RenderQueueSingleton::IsInitialized())
				RenderQueueSingleton::Get()->AddRenderFront(model.get());

			ScreenSprite* rawPtr = dynamic_cast<ScreenSprite*>(model.get());
			if (rawPtr)
			{
				m_FrontSprites[rawPtr->GetAssignedID()] = std::unique_ptr<ScreenSprite>(rawPtr);
				model.release(); // prevent double-delete
			}
			else
			{
				LOG_ERROR("Failed to cast IRender to Front Sprite for object: " + key.first);
			}
		}
		else
		{
			LOG_ERROR("Unknown type: " + type + " — not handled in LoadLevel");
		}
	}
}

void LevelEditor::SaveObjects()
{
	//~ Save all the data
	for (auto& render : m_Renders | std::views::values)
	{
		if (!render) continue;

		std::string key = render->GetName() + "##" + std::to_string(render->GetAssignedID());
		m_LevelData.GetOrCreate("ObjectData").GetOrCreate(key) = render->GetSweetData();
	}

	//~ Save all the data
	for (auto& render : m_BackgroundSprites | std::views::values)
	{
		if (!render) continue;

		std::string key = render->GetName() + "##" + std::to_string(render->GetAssignedID());
		m_LevelData.GetOrCreate("ObjectData").GetOrCreate(key) = render->GetSweetData();
	}

	//~ Save all the data
	for (auto& render : m_FrontSprites | std::views::values)
	{
		if (!render) continue;

		std::string key = render->GetName() + "##" + std::to_string(render->GetAssignedID());
		m_LevelData.GetOrCreate("ObjectData").GetOrCreate(key) = render->GetSweetData();
	}
}

void LevelEditor::LoadLights()
{
	for (auto& lightData : m_LevelData["LightData"] | std::views::values)
	{
		std::string type = lightData["LightType"].GetValue();
		if (type.empty()) continue;

		std::unique_ptr<ILightSource> light = RegistryLight::Create(type);
		if (!light) continue;

		light->SetSweetData(lightData);

		if (RenderQueueSingleton::IsInitialized())
			RenderQueueSingleton::Get()->AddLight(light.get());

		m_LightSources[light->GetAssignedID()] = std::move(light);
	}
}

void LevelEditor::SaveLights()
{
	//~ Save all the data
	for (auto& light : m_LightSources | std::views::values)
	{
		if (!light) continue;

		std::string key = light->GetLightName() + "##" + std::to_string(light->GetAssignedID());
		m_LevelData.GetOrCreate("LightData").GetOrCreate(key) = light->GetSweetData();
	}
}

void LevelEditor::LoadCameraConfig()
{
	if (RenderQueueSingleton::IsInitialized())
	{
		RenderQueueSingleton::Get()->GetCameraController()->SetSweetData(m_LevelData["CameraData"]);
	}
}

void LevelEditor::SaveCameraConfig()
{
	if (RenderQueueSingleton::IsInitialized())
	{
		auto data = RenderQueueSingleton::Get()->GetCameraController()->GetSweetData();
		m_LevelData.GetOrCreate("CameraData") = data;
	}
}

void LevelEditor::RenderMenuUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Edit Object")) m_bDisplayEditObjectUI = !m_bDisplayEditObjectUI;
			if (ImGui::MenuItem("Rendered Object")) m_bDisplayRenderObjectUI = !m_bDisplayRenderObjectUI;

			if (ImGui::BeginMenu("Sprites"))
			{
				if (ImGui::MenuItem("Background Sprites")) m_bDisplayBackgroundObjectUI = !m_bDisplayBackgroundObjectUI;
				if (ImGui::MenuItem("Front Sprites")) m_bDisplayFrontObjectUI = !m_bDisplayFrontObjectUI;
				if (ImGui::MenuItem("Space Sprites")) m_bDisplaySpaceObjectUI = !m_bDisplaySpaceObjectUI;
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Light")) m_bDisplayLightUI = !m_bDisplayLightUI;
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::BeginMenu("Mesh"))
			{
				if (ImGui::MenuItem("Cube")) m_bCreateCubeRenderObjectUI = true;
				if (ImGui::MenuItem("OBJ")) m_bCreateOBJRenderObjectUI = true;
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Sprites"))
			{
				if (ImGui::MenuItem("Background Sprite")) m_bCreateBackgroundRenderObjectUI = true;
				if (ImGui::MenuItem("Front Sprite")) m_bCreateFrontRenderObjectUI = true;
				if (ImGui::MenuItem("Space Sprite")) m_bCreateSpaceRenderObjectUI = true;
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Light"))
			{
				if (ImGui::MenuItem("Directional Light")) m_bCreateDirectionalLightUI = true;
				if (ImGui::MenuItem("Spot Light")) m_bCreateSpotLightUI = true;
				if (ImGui::MenuItem("Point Light")) m_bCreatePointLightUI = true;
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	//~ Object to Edit
	RenderEditControlUI();

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

	//~ Lights
	RenderLightControlUI();
	RenderDirectionalLightCreationUI();
	RenderSpotLightCreationUI();
	RenderPointLightCreationUI();
}

void LevelEditor::RenderEditControlUI() const
{
	if (!m_bDisplayEditObjectUI) return;
	if (!RenderQueueSingleton::IsInitialized()) return;

	CameraController* camera = RenderQueueSingleton::Get()->GetCameraController();

	ImGui::Begin("Render Edit Object Controls");

	for (auto& [id, render] : m_AttachedToEdit)
	{
		if (!render || !RenderQueueSingleton::Get()->IsInside(render))
			continue;

		std::string label = render->GetName() + "##" + std::to_string(id);

		if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::PushID(static_cast<int>(id));

			bool isAttached = (camera->IsCameraAttachedToObject() && camera->GetAttachedObject() == render);

			if (ImGui::Checkbox("Attach Camera", &isAttached))
			{
				if (isAttached) camera->AttachCameraToObject(render);
				else if (camera->GetAttachedObject() == render)
					camera->DetachCameraFromObject();
			}

			if (camera->IsCameraAttachedToObject() && camera->GetAttachedObject() == render)
			{
				bool follow = camera->IsFollowingAttached();
				if (ImGui::Checkbox("Follow Object", &follow))
					camera->FollowAttached(follow);

				bool lookAt = camera->IsLookingAtAttached();
				if (ImGui::Checkbox("Look At Object", &lookAt))
					camera->LookAtAttached(lookAt);

				DirectX::XMFLOAT3 offset = camera->GetOffsetToAttach();
				if (ImGui::DragFloat3("Camera Offset", &offset.x, 0.1f))
					camera->SetOffsetToAttached(offset);
			}

			ImGui::Separator();
			render->RenderControlUI();
			ImGui::PopID();
		}
	}

	ImGui::End();
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
		auto cube = RegistryMesh::Create("ModelCube");

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

	CameraController* camera = RenderQueueSingleton::Get()->GetCameraController();

	ImGui::Begin("Render Object Controls");

	for (auto& [id, render] : m_Renders)
	{
		if (!render )
			continue;

		std::string label = render->GetName() + "##" + std::to_string(id);

		if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::PushID(static_cast<int>(id));

			bool isAttached = (camera->IsCameraAttachedToObject() && camera->GetAttachedObject() == render.get());

			if (ImGui::Checkbox("Attach Camera", &isAttached))
			{
				if (isAttached) camera->AttachCameraToObject(render.get());
				else if (camera->GetAttachedObject() == render.get())
					camera->DetachCameraFromObject();
			}

			if (camera->IsCameraAttachedToObject() && camera->GetAttachedObject() == render.get())
			{
				bool follow = camera->IsFollowingAttached();
				if (ImGui::Checkbox("Follow Object", &follow))
					camera->FollowAttached(follow);

				bool lookAt = camera->IsLookingAtAttached();
				if (ImGui::Checkbox("Look At Object", &lookAt))
					camera->LookAtAttached(lookAt);

				DirectX::XMFLOAT3 offset = camera->GetOffsetToAttach();
				if (ImGui::DragFloat3("Camera Offset", &offset.x, 0.1f))
					camera->SetOffsetToAttached(offset);
			}

			ImGui::Separator();
			render->RenderControlUI();
			ImGui::PopID();
		}
	}

	ImGui::End();
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
			m_HolderMesh = std::unique_ptr<Mesh>(
				dynamic_cast<Mesh*>(RegistryMesh::Create("Mesh").release())
			);
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
			}else
			{
				m_HolderMesh.reset();
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
			m_BackgroundHolderSprite = std::unique_ptr<BackgroundSprite>(
				dynamic_cast<BackgroundSprite*>(RegistryMesh::Create("BackgroundSprite").release())
			);
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
			}else
			{
				m_BackgroundHolderSprite.reset();
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
			m_FrontHolderSprite = std::unique_ptr<ScreenSprite>(
				dynamic_cast<ScreenSprite*>(RegistryMesh::Create("ScreenSprite").release())
			);
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
			}else
			{
				m_FrontHolderSprite.reset();
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
			m_SpaceHolderSprite = std::unique_ptr<WorldSpaceSprite>(
				dynamic_cast<WorldSpaceSprite*>(RegistryMesh::Create("WorldSpaceSprite").release())
			);
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
			}else
			{
				m_SpaceHolderSprite.reset();
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

void LevelEditor::RenderLightControlUI() const
{
	if (!m_bDisplayLightUI) return;

	if (!RenderQueueSingleton::IsInitialized()) return;

	ImGui::Begin("Light Controls"); // All controls go under this one window

	for (auto& [id, light] : RenderQueueSingleton::Get()->GetLights())
	{
		if (!light) continue;

		std::string labelTitle = light->GetLightName() + " [" + light->GetLightTypeToString() + "]";
		std::string label = labelTitle + "##" + std::to_string(id);

		if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::PushID(static_cast<int>(id));
			light->RenderControlUI();
			ImGui::PopID();
		}
	}
	ImGui::End(); // End of main window
}

void LevelEditor::RenderDirectionalLightCreationUI()
{
	if (m_bCreateDirectionalLightUI)
	{
		ImGui::OpenPopup(m_RenderDirectionalLightPopUpName.c_str());
		m_bCreateDirectionalLightUI = false;
	}

	if (ImGui::BeginPopupModal(m_RenderDirectionalLightPopUpName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (m_DirectionalLightHolder == nullptr)
		{
			m_DirectionalLightHolder = std::unique_ptr<DirectionalLight>(
				dynamic_cast<DirectionalLight*>(RegistryLight::Create("DirectionalLight").release())
			);
		}
		m_DirectionalLightHolder->RenderControlUI();

		if (ImGui::Button("Create"))
		{
			if (m_DirectionalLightHolder->IsInitialized())
			{
				RenderQueueSingleton::Get()->AddLight(m_DirectionalLightHolder.get());
				m_LightSources[m_DirectionalLightHolder->GetAssignedID()] = std::move(m_DirectionalLightHolder);
				m_DirectionalLightHolder = nullptr;
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

void LevelEditor::RenderPointLightCreationUI()
{
	if (m_bCreatePointLightUI)
	{
		ImGui::OpenPopup(m_RenderPointLightPopUpName.c_str());
		m_bCreatePointLightUI = false;
	}

	if (ImGui::BeginPopupModal(m_RenderPointLightPopUpName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (m_PointLightHolder == nullptr)
		{
			m_PointLightHolder = std::unique_ptr<PointLight>(
				dynamic_cast<PointLight*>(RegistryLight::Create("PointLight").release())
			);
		}
		m_PointLightHolder->RenderControlUI();

		if (ImGui::Button("Create"))
		{
			if (m_PointLightHolder->IsInitialized())
			{
				RenderQueueSingleton::Get()->AddLight(m_PointLightHolder.get());
				m_LightSources[m_PointLightHolder->GetAssignedID()] = std::move(m_PointLightHolder);
				m_PointLightHolder = nullptr;
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

void LevelEditor::RenderSpotLightCreationUI()
{
	if (m_bCreateSpotLightUI)
	{
		ImGui::OpenPopup(m_RenderSpotLightPopUpName.c_str());
		m_bCreateSpotLightUI = false;
	}

	if (ImGui::BeginPopupModal(m_RenderSpotLightPopUpName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (m_SpotLightHolder == nullptr)
		{
			m_SpotLightHolder = std::unique_ptr<SpotLight>(
				dynamic_cast<SpotLight*>(RegistryLight::Create("SpotLight").release())
			);
		}

		m_SpotLightHolder->RenderControlUI();

		if (ImGui::Button("Create"))
		{
			if (m_SpotLightHolder->IsInitialized())
			{
				RenderQueueSingleton::Get()->AddLight(m_SpotLightHolder.get());
				m_LightSources[m_SpotLightHolder->GetAssignedID()] = std::move(m_SpotLightHolder);
				m_SpotLightHolder = nullptr;
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
