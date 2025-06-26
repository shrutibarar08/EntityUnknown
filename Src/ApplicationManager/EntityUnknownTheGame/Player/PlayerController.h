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

	void OnBeginPlay(const SweetLoader& Config) override;
	void OnTick(float deltaTime) override;

	void SaveSweetData(SweetLoader& sweetLoader) override;
	IRender* GetActorMesh() const override;

	void HandleInput(float deltaTime) override;
	void OnFocus() override;
	void OffFocus() override;

	void SetCameraOffset(const DirectX::XMFLOAT3& offset);
	DirectX::XMFLOAT3 GetCameraOffset() const;

private:
	SweetLoader m_PlayerData{};
	std::unique_ptr<IRender> m_PlayerMesh{ nullptr };

	//~ Controller Specific
	bool m_bCameraOffsetDirty{ true };
	DirectX::XMFLOAT3 m_CameraOffset{ 0, 0, -10 };
};
