#pragma once
#include "ApplicationManager/IApplication.h"
#include "Enemies/Ghost/EnemyGhost.h"
#include "Player/PlayerController.h"

#define DEFAULT_GAME_DATA_PATH "Data/GameConfig.json"

enum class GameState: uint8_t
{
	MENU,
	PLAYING,
	DEAD,
	WON
};

class EntityUnknownTheGame: public IApplication
{
public:
	EntityUnknownTheGame() = default;
	~EntityUnknownTheGame() override = default;

protected:
	bool InitializeApplication(const SweetLoader& sweetLoader) override;
	void Update(float deltaTime) override;
	void SaveSweetData(SweetLoader& sweetLoader) override;

private:
	//~ Game Related Data
	SweetLoader m_GameData{};
	GameState m_GameSate{ GameState::PLAYING };
};
