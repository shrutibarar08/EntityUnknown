#include "EntityUnknownTheGame.h"

#include <iostream>


bool EntityUnknownTheGame::InitializeApplication(const SweetLoader& sweetLoader)
{
	//~ Load Game Data
	std::string dataPath = sweetLoader["EntityUnknownTheGame"]["DataPath"].GetValue();
	if (dataPath.empty()) dataPath = DEFAULT_GAME_DATA_PATH;

	m_GameData.Load(dataPath);
	m_GameData.GetOrCreate("DataPath") = dataPath;

	//~ Load Player Data
	m_Player = std::make_unique<PlayerController>();
	m_Player->OnBeginPlay(m_GameData.GetOrCreate("PlayerDataPath"));
	m_LevelEditor->LoadLevel(m_GameData.GetOrCreate("LevelDataPath"));
#ifdef _DEBUG
	m_LevelEditor->AttachRenderToEdit(m_Player->GetActorMesh());
#endif

	return true;
}

void EntityUnknownTheGame::Update(float deltaTime)
{
	if (m_Player) m_Player->OnTick(deltaTime);
}

void EntityUnknownTheGame::OnQuit(SweetLoader& sweetLoader)
{
	//~ Save Game data
	std::string dataPath = m_GameData["DataPath"].GetValue();
	if (dataPath.empty()) dataPath = DEFAULT_GAME_DATA_PATH;
	sweetLoader.GetOrCreate("EntityUnknownTheGame").GetOrCreate("DataPath") = dataPath;

	//~ Save Player Data
	if (m_Player) m_Player->SaveSweetData(m_GameData.GetOrCreate("PlayerDataPath"));

	//~ Save Level Data
	if (m_LevelEditor) m_LevelEditor->SaveSweetData(m_GameData.GetOrCreate("LevelDataPath"));

	//~ Save
	m_GameData.Save(dataPath);
}
