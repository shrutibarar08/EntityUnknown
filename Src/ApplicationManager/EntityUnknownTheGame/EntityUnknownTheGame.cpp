#include "EntityUnknownTheGame.h"

bool EntityUnknownTheGame::InitializeApplication(const SweetLoader& sweetLoader)
{
	//~ Load Game Data
	std::string dataPath = sweetLoader["EntityUnknownTheGame"]["DataPath"].GetValue();
	if (dataPath.empty()) dataPath = DEFAULT_GAME_DATA_PATH;

	m_GameData.Load(dataPath);
	m_GameData.GetOrCreate("DataPath") = dataPath;

#ifdef _DEBUG
	//m_InputHandler->AddInputController(m_LevelEditor.get());
	//m_InputHandler->FocusControlOn(m_LevelEditor->GetAssignedID());
#endif

	return true;
}

void EntityUnknownTheGame::Update(float deltaTime)
{
	if (m_WindowsSystem->Keyboard.WasKeyPressed(VK_F1))
	{
	}

#ifdef _DEBUG
	if (m_WindowsSystem->Keyboard.WasKeyPressed(VK_F2))
	{
		if (ID id = m_InputHandler->GetFocusedOnID(); id != 0)
		{
		}
	}
#endif
}

void EntityUnknownTheGame::SaveSweetData(SweetLoader& sweetLoader)
{

}
