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

class LevelEditor final: public ISystem, public ISystemRender, public IInputContext
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
	void SpawnPlayer() const;

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
	void RenderHealthSprites();
	void EditThings();

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
	void RenderPlayerControlUI();
	void RenderPlayerMeshUI() const;
	void RenderPlayerInputControlUI();
	void RenderPlayerAnimStates();

public:
	void HandleInput(float deltaTime) override;

private:
	//~ Input Handler
	// Setters
	void HandleMouseLook(float deltaTime) const;

	void SetMoveForwardKey(KeyCode key) { m_MoveForwardKey = key; }
	void SetMoveBackwardKey(KeyCode key) { m_MoveBackwardKey = key; }
	void SetMoveLeftKey(KeyCode key) { m_MoveLeftKey = key; }
	void SetMoveRightKey(KeyCode key) { m_MoveRightKey = key; }

	void SetMouseSensitivityX(float x) { m_MouseSensitivityX = x; }
	void SetMouseSensitivityY(float y) { m_MouseSensitivityY = y; }
	void SetMouseOnScreen(bool val);
	bool IsMouseOnScreen() const { return m_ThirdPersonView; }

	// Getters
	KeyCode GetMoveForwardKey()     const { return m_MoveForwardKey; }
	KeyCode GetMoveBackwardKey()    const { return m_MoveBackwardKey; }
	KeyCode GetMoveLeftKey()        const { return m_MoveLeftKey; }
	KeyCode GetMoveRightKey()       const { return m_MoveRightKey; }

	float GetMouseSensitivityX() const { return m_MouseSensitivityX; }
	float GetMouseSensitivityY() const { return m_MouseSensitivityY; }

	KeyCode m_MoveForwardKey{ 'W' };
	KeyCode m_MoveBackwardKey{ 'S' };
	KeyCode m_MoveLeftKey{ 'A' };
	KeyCode m_MoveRightKey{ 'D' };
	float m_MouseSensitivityX = 0.8f;
	float m_MouseSensitivityY = 0.8f;
	bool m_ThirdPersonView{ false };

private:
	//~ Health Config
	// Sprite Location
	DirectX::XMFLOAT3 m_SpriteStartPosition{};
	int m_XSpritePadding = 0;

	float m_LeftPercentage = 0;
	float m_RightPercentage = 0;
	float m_TopPercentage = 0;
	float m_BottomPercentage = 0;

	struct HeathRenderStatus
	{
		std::unique_ptr<ScreenSprite> m_HealthSprite;
		bool m_Rendering;
	};
	std::unordered_map<int, HeathRenderStatus> m_healthSprite{};

	//~ Player Config
	bool m_bDisplayPlayerUI{ false };
	PlayerController* m_PlayerController{ nullptr };

	DirectX::XMFLOAT3 m_PlayerStartPosition{ 0.f, 0.f, 0.f};

	//~ Config Editor
	SweetLoader m_LevelData{};
	std::unordered_map<ID, IRender*> m_AttachedToEdit{};
	SweetLoader m_LevelEditorConfig{};
	bool m_bDisplayEditObjectUI{ false };

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
