#pragma once
#include "ApplicationManager/EntityUnknownTheGame/Player/PlayerController.h"
#include "RenderManager/IRender.h"
#include "RenderManager/ISystemRender.h"
#include "RenderManager/Model/Mesh/Mesh.h"
#include "RenderManager/Sprite/BackgroundSprite/BackgroundSprite.h"
#include "RenderManager/Sprite/ScreenSprite/ScreenSprite.h"
#include "RenderManager/Sprite/WorldSpaceSprite/WorldSpaceSprite.h"
#include "SystemManager/ISystem.h"

#include "RenderManager/DefineRenders.h"
#include "RenderManager/Light/DefineLights.h"

#define DEFAULT_LEVEL_DATA_PATH "Data/Level/level.json"

class LevelEditor final: public ISystem, public ISystemRender
{
public:
	LevelEditor() = default;
	~LevelEditor() override = default;

	LevelEditor(const LevelEditor&) = delete;
	LevelEditor(LevelEditor&&) = delete;
	LevelEditor& operator=(const LevelEditor&) = delete;
	LevelEditor& operator=(LevelEditor&&) = delete;

	//~ For initializing Level Editor
	bool OnInit(const SweetLoader& sweetLoader) override;
	bool OnFrameUpdate(float deltaTime) override;
	bool OnFrameClear() override;
	bool OnExit(SweetLoader& sweetLoader) override;
	std::string GetSystemName() override;

	//~ For ImGui or system level rendering
	void RenderBegin() override;
	void RenderExecute() override;
	void RenderEnd() override;

	void AttachRenderToEdit(IRender* render);
	SweetLoader GetLevelConfig() const;

	void LoadLevel(const SweetLoader& sweetLevelData);
	void SaveSweetData(SweetLoader& data);

	void AttachPlayer(PlayerController* playerController);

private:
	void LoadObjects();
	void SaveObjects();
		
	void LoadLights();
	void SaveLights();

	void LoadCameraConfig();
	void SaveCameraConfig();

private:

	//~ UI
	void RenderMenuUI();

	//~ Edit Objects
	void RenderEditControlUI() const;

	//~ 3D Mesh thingy
	void RenderObjectCubeCreationUI();
	void Render3DObjectControlsUI() ;
	void RenderOBJCreationUI();

	//~ Background thingy
	void RenderBackgroundSpriteControlUI();
	void RenderBackgroundSpriteCreationUI();

	//~ Front Thingy
	void RenderFrontSpriteControlUI();
	void RenderFrontSpriteCreationUI();

	//~ Space Sprite Thingy
	void RenderSpaceSpriteControlUI();
	void RenderSpaceSpriteCreationUI();

	//~ Lights Related Thingy
	void RenderLightControlUI();
	void RenderDirectionalLightCreationUI();
	void RenderPointLightCreationUI();
	void RenderSpotLightCreationUI();

	//~ Player
	void RenderPlayerControlUI() const;
	void RenderPlayerMeshUI() const;
	void RenderPlayerCameraUI() const;

private:
	//~ Config Editor
	SweetLoader m_LevelData{};
	std::unordered_map<ID, IRender*> m_AttachedToEdit{};
	SweetLoader m_LevelEditorConfig{};
	bool m_bDisplayEditObjectUI{ false };
	bool m_bDisplayPlayerUI{ false };
	PlayerController* m_PlayerController{ nullptr };

	//~ Renders
	std::unordered_map<ID, std::unique_ptr<IRender>> m_Renders{};
	bool m_bDisplayRenderObjectUI{ false };
	bool m_bCreateCubeRenderObjectUI{ false };

	//~ Display and Render OBJ Models
	std::string m_RenderOBJPopUpName{ "Create OBJ Object" };
	bool m_bCreateOBJRenderObjectUI{ false };
	std::unique_ptr<Mesh> m_HolderMesh{ nullptr };

	//~ Display Background Creation related things
	std::unordered_map<ID, std::unique_ptr<BackgroundSprite>> m_BackgroundSprites{};
	std::string m_RenderBackgroundPopUpName{ "Create background Sprite" };
	bool m_bCreateBackgroundRenderObjectUI{ false };
	bool m_bDisplayBackgroundObjectUI{ false };
	std::unique_ptr<BackgroundSprite> m_BackgroundHolderSprite{ nullptr };

	//~ Display Front sprite related things
	std::unordered_map<ID, std::unique_ptr<ScreenSprite>> m_FrontSprites{};
	std::string m_RenderFrontPopUpName{ "Create Front Sprite" };
	bool m_bCreateFrontRenderObjectUI{ false };
	bool m_bDisplayFrontObjectUI{ false };
	std::unique_ptr<ScreenSprite> m_FrontHolderSprite{ nullptr };

	//~ Display Space sprite related things
	std::unordered_map<ID, std::unique_ptr<WorldSpaceSprite>> m_SpaceSprites{};
	std::string m_RenderSpacePopUpName{ "Create Space Sprite" };
	bool m_bCreateSpaceRenderObjectUI{ false };
	bool m_bDisplaySpaceObjectUI{ false };
	std::unique_ptr<WorldSpaceSprite> m_SpaceHolderSprite{ nullptr };

	//~ Light
	std::unordered_map<ID, std::unique_ptr<ILightSource>> m_LightSources{};
	bool m_bDisplayLightUI{ false };

	std::string m_RenderDirectionalLightPopUpName{"Create Directional Light"};
	std::string m_RenderSpotLightPopUpName{"Create Spot Light"};
	std::string m_RenderPointLightPopUpName{"Create Point Light"};

	bool m_bCreateDirectionalLightUI{ false };
	bool m_bCreatePointLightUI{ false };
	bool m_bCreateSpotLightUI{ false };

	std::unique_ptr<ILightSource> m_DirectionalLightHolder{ nullptr };
	std::unique_ptr<SpotLight> m_SpotLightHolder{ nullptr };
	std::unique_ptr<PointLight> m_PointLightHolder{ nullptr };
};
