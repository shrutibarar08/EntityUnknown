#pragma once
#include "ApplicationManager/EntityUnknownTheGame/Interface/IActor.h"
#include "ApplicationManager/InputHandler/InputHandler.h"
#include "RenderManager/Animation/SpriteAnimator/SpriteAnimStateMachine.h"
#include "RenderManager/Sprite/ScreenSprite/ScreenSprite.h"
#include "RenderManager/Sprite/WorldSpaceSprite/WorldSpaceSprite.h"

#define DEFAULT_PLAYER_DATA_PATH "Data/PlayerConfig.json"

enum class PlayerAnimState: uint8_t
{
	IDLE,
	WALKING_LEFT,
	WALKING_RIGHT,
	JUMPING
};

class PlayerController final: public virtual IActor, public virtual IInputContext
{
public:
	PlayerController();
	~PlayerController() override = default;

	PlayerController(const PlayerController&) = delete;
	PlayerController(PlayerController&&) = delete;
	PlayerController& operator=(const PlayerController&) = delete;
	PlayerController& operator=(PlayerController&&) = delete;

	void BuildCheck(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	void OnBeginPlay(const SweetLoader& sweetData) override;
	void OnTick(float deltaTime) override;

	void SaveSweetData(SweetLoader& sweetLoader) override;
	IRender* GetActorMesh() const override;

	void HandleInput(float deltaTime) override;
	void OnFocus() override;
	void OffFocus() override;

	void SetCameraOffset(const DirectX::XMFLOAT3& offset);
	DirectX::XMFLOAT3 GetCameraOffset() const;

	float GetRunningSpeed() const { return m_RunningSpeed; }
	float GetMaxRunningSpeed() const { return m_MaxRunningVelocityX; }
	float GetJumpingForce() const { return m_JumpingForce; }

	void SetRunningSpeed(float value) { m_RunningSpeed = value; }
	void SetMaxRunningSpeed(float value) { m_MaxRunningVelocityX = value; }
	void SetJumpingForce(float value) { m_JumpingForce = value; }

	SpriteAnimStateMachine* GetPlayerAnimState() const;

	//~ Helpers
	static const char* ToString(PlayerAnimState state);
	static PlayerAnimState PlayerAnimStateFromString(const std::string& str);

	bool IsPlayerDead() const { return m_HealthBar == 0; }
	void HurtPlayer(int hurtValue);
	void PlayerLifeReset() { m_HealthBar = m_MaxHeath; }

	int GetPlayerHeath() const { return m_HealthBar; }

private:
	void LoadInputControls(const SweetLoader& sweetData);
	void SaveInputControls();

	//~ Handle Inputs
	void PlayerInput(float deltaTime) const;

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

	float m_RunningSpeed{ 30.0f };
	float m_MaxRunningVelocityX{ 5.f };
	float m_JumpingForce{ 5.0f };
};
