#pragma once
#include "ApplicationManager/IApplication.h"
#include "Player/PlayerController.h"

#define DEFAULT_GAME_DATA_PATH "Data/GameConfig.json"

class EntityUnknownTheGame: public IApplication
{
public:
	EntityUnknownTheGame() = default;
	~EntityUnknownTheGame() override = default;

protected:
	bool InitializeApplication(const SweetLoader& sweetLoader) override;
	void Update(float deltaTime) override;
	void OnQuit(SweetLoader& sweetLoader) override;

private:
	std::unique_ptr<PlayerController> m_Player{ nullptr };
	SweetLoader m_GameData{};
};
