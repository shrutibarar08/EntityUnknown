#include "PlayerController.h"

#include "RenderManager/RenderQueue/RenderQueue.h"


PlayerController::PlayerController()
{
	m_PlayerMesh = std::make_unique<WorldSpaceSprite>();
	m_PlayerMesh->SetTransparent(true);
}

void PlayerController::OnBeginPlay(const SweetLoader& Config)
{
	std::string playerData = Config["PlayerDataPath"].GetValue();
	if (playerData.empty()) playerData = DEFAULT_PLAYER_DATA_PATH;
	m_PlayerData.Load(playerData);

	if (m_PlayerMesh)
	{
		m_PlayerMesh->SetSweetData(m_PlayerData.GetOrCreate("RigidBody"));
		if (RenderQueueSingleton::IsInitialized())
		{
			RenderQueueSingleton::Get()->AddRender(m_PlayerMesh.get());
		}
	}
	m_PlayerData.GetOrCreate("PlayerDataPath") = playerData;
}

void PlayerController::OnTick(float deltaTime)
{
	IActor::OnTick(deltaTime);
}

void PlayerController::SaveSweetData(SweetLoader& config)
{
	std::string playerData = m_PlayerData["PlayerDataPath"].GetValue();
	if (playerData.empty()) playerData = DEFAULT_PLAYER_DATA_PATH;

	config.GetOrCreate("PlayerDataPath") = playerData;

	if (m_PlayerMesh)
	{
		m_PlayerData.GetOrCreate("RigidBody") = m_PlayerMesh->GetSweetData();
	}

	m_PlayerData.Save(playerData);
}

IRender* PlayerController::GetActorMesh() const
{
	if (m_PlayerMesh) return m_PlayerMesh.get();
	return nullptr;
}
