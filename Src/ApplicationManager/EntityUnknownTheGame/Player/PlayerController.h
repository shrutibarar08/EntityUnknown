#pragma once
#include "ApplicationManager/EntityUnknownTheGame/Interface/IActor.h"
#include "RenderManager/Sprite/WorldSpaceSprite/WorldSpaceSprite.h"

#define DEFAULT_PLAYER_DATA_PATH "Data/PlayerConfig.json"

class PlayerController final: public IActor
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

private:
	SweetLoader m_PlayerData{};
	std::unique_ptr<IRender> m_PlayerMesh{ nullptr };
};
