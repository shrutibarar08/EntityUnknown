#include "EntityUnknownTheGame.h"

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

	//~ Init Enemies
	m_EnemyGhost = std::make_unique<EnemyGhost>();
	m_EnemyGhost->SetEnemyPath("GhostEnemy.json");
	m_EnemyGhost->OnBeginPlay(m_GameData.GetOrCreate("EnemyPath"));

	m_LevelEditor->LoadLevel(m_GameData.GetOrCreate("LevelDataPath"));
	m_LevelEditor->AttachPlayer(m_Player.get());
	m_LevelEditor->AttachActor(m_EnemyGhost.get());

	m_InputHandler->AddInputController(m_Player.get());
	m_InputHandler->FocusControlOn(m_Player->GetAssignedID());

#ifdef _DEBUG
	m_InputHandler->AddInputController(m_LevelEditor.get());
	m_InputHandler->FocusControlOn(m_LevelEditor->GetAssignedID());
#endif


	//~ Creating death fall
	m_DeathFall = std::make_unique<ModelCube>();
	m_DeathFall->SetSweetData(m_GameData.GetOrCreate("DeathFall"));

	//~ Configure Death Fall
	m_DeathFall->GetCubeCollider()->SetColliderState(ColliderState::Trigger);

	TRIGGER_COLLISION_INFO deathFallTriggerInfo{};
	deathFallTriggerInfo.TargetCollider = m_Player->GetActorMesh()->GetCubeCollider();
	deathFallTriggerInfo.m_OnTriggerEnterCallbackFn = [&]()
	{
		LOG_INFO("Fall Detected!");
		m_Player->HurtPlayer(1);
		m_LevelEditor->SpawnPlayer();
	};
	deathFallTriggerInfo.m_OnTriggerExitCallbackFn = {};
	m_DeathFall->GetCubeCollider()->SetTriggerTarget(deathFallTriggerInfo);

	TRIGGER_COLLISION_INFO info{};
	info.TargetCollider = m_Player->GetActorMesh()->GetCubeCollider();
	info.m_OnTriggerEnterCallbackFn = [&]()
	{
		LOG_INFO("Caught");
		m_Player->HurtPlayer(1);
		if (!m_Player->IsPlayerDead()) m_LevelEditor->SpawnPlayer();
	};
	m_EnemyGhost->GetActorMesh()->GetCubeCollider()->SetTriggerTarget(info);

	m_LevelEditor->AttachRenderToEdit(m_DeathFall.get());
	m_LevelEditor->SpawnPlayer();

	return true;
}

void EntityUnknownTheGame::Update(float deltaTime)
{
	if (m_Player) m_Player->OnTick(deltaTime);
	if (m_EnemyGhost) m_EnemyGhost->OnTick(deltaTime);

	if (m_WindowsSystem->Keyboard.WasKeyPressed(VK_F1))
	{
		SaveSweetData(m_Config);
		LOG_INFO("Saving Data after f1");
	}

#ifdef _DEBUG
	if (m_WindowsSystem->Keyboard.WasKeyPressed(VK_F2))
	{
		if (ID id = m_InputHandler->GetFocusedOnID(); id != 0)
		{
			if (id == m_LevelEditor->GetAssignedID()) m_InputHandler->FocusControlOn(m_Player->GetAssignedID());
			else m_InputHandler->FocusControlOn(m_LevelEditor->GetAssignedID());
		}
	}
#endif
}

void EntityUnknownTheGame::SaveSweetData(SweetLoader& sweetLoader)
{
	//~ Save Game data
	std::string dataPath = m_GameData["DataPath"].GetValue();
	if (dataPath.empty()) dataPath = DEFAULT_GAME_DATA_PATH;
	sweetLoader.GetOrCreate("EntityUnknownTheGame").GetOrCreate("DataPath") = dataPath;

	//~ Save Player Data
	if (m_Player) m_Player->SaveSweetData(m_GameData.GetOrCreate("PlayerDataPath"));
	if (m_EnemyGhost) m_EnemyGhost->SaveSweetData(m_GameData.GetOrCreate("EnemyPath"));

	//~ Save Level Data
	if (m_LevelEditor) m_LevelEditor->SaveSweetData(m_GameData.GetOrCreate("LevelDataPath"));

	if (m_DeathFall) m_GameData.GetOrCreate("DeathFall") = m_DeathFall->GetSweetData();

	//~ Save
	m_GameData.Save(dataPath);
}
