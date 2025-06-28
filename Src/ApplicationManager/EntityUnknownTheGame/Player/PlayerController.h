#pragma once
#include "ApplicationManager/EntityUnknownTheGame/Interface/IActor.h"
#include "ApplicationManager/InputHandler/InputHandler.h"
#include "RenderManager/Animation/SpriteAnimator/SpriteAnimStateMachine.h"
#include "RenderManager/Sprite/ScreenSprite/ScreenSprite.h"
#include "RenderManager/Sprite/WorldSpaceSprite/WorldSpaceSprite.h"

#define DEFAULT_PLAYER_DATA_PATH "Data/PlayerConfig.json"


class PlayerController final: public virtual IActor, public virtual IInputContext
{
public:
	PlayerController();
	~PlayerController() override = default;

	PlayerController(const PlayerController&) = delete;
	PlayerController(PlayerController&&) = delete;
	PlayerController& operator=(const PlayerController&) = delete;
	PlayerController& operator=(PlayerController&&) = delete;

	void BuildCheck(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

	void OnBeginPlay(const SweetLoader& sweetData) override;
	void OnTick(float deltaTime) override;

	void SaveSweetData(SweetLoader& sweetLoader) override;
	IRender* GetActorMesh() const override;

	void HandleInput(float deltaTime) override;
	void OnFocus() override;
	void OffFocus() override;

	void SetCameraOffset(const DirectX::XMFLOAT3& offset);
	DirectX::XMFLOAT3 GetCameraOffset() const;

	SpriteAnimStateMachine* GetAnimState() const override;

	bool IsPlayerDead() const { return m_HealthBar <= 0; }
	void HurtPlayer(int hurtValue);
	void PlayerLifeReset() { m_HealthBar = m_MaxHeath; }

	int GetPlayerHeath() const { return m_HealthBar; }

private:
	void LoadInputControls(const SweetLoader& sweetData);
	void SaveInputControls();

	//~ Handle Inputs
	void PlayerInput(float deltaTime);

	void CameraInput(float deltaTime);

private:

	int m_MaxHeath{ 3 };
	int m_HealthBar{ 3 };

	SweetLoader m_PlayerData{};
	std::unique_ptr<IRender> m_PlayerMesh{ nullptr };
	std::unique_ptr<SpriteAnimStateMachine> m_PlayerAnimation{ nullptr };

	//~ Controller Specific
	bool m_bCameraOffsetDirty{ true };
	DirectX::XMFLOAT3 m_CameraOffset{ 0, 0, -10 };
};
