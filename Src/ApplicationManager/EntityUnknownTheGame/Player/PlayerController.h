#pragma once
#include "ApplicationManager/EntityUnknownTheGame/Interface/IActor.h"
#include "ApplicationManager/InputHandler/InputHandler.h"
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
	float GetJumpingForce() const { return m_JumpingForce; }

	void SetRunningSpeed(float value) { m_RunningSpeed = value; }
	void SetJumpingForce(float value) { m_JumpingForce = value; }

private:
	void LoadInputControls(const SweetLoader& sweetData);
	void SaveInputControls();

private:
	SweetLoader m_PlayerData{};
	std::unique_ptr<IRender> m_PlayerMesh{ nullptr };

	//~ Controller Specific
	bool m_bCameraOffsetDirty{ true };
	DirectX::XMFLOAT3 m_CameraOffset{ 0, 0, -10 };
	float m_RunningSpeed{ 30.0f };
	float m_JumpingForce{ 5.0f };
};
