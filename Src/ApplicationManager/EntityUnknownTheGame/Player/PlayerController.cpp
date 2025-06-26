#include "PlayerController.h"

#include "RenderManager/RenderQueue/RenderQueue.h"


PlayerController::PlayerController()
{
	m_PlayerMesh = std::make_unique<WorldSpaceSprite>();
	m_PlayerMesh->SetTransparent(true);
	m_PlayerMesh->GetShaderResource()->SetTexture("Texture/idle/0_Reaper_Man_Idle_001.tga");
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

void PlayerController::SaveSweetData(SweetLoader& sweetLoader)
{
	std::string playerData = m_PlayerData["PlayerDataPath"].GetValue();
	if (playerData.empty()) playerData = DEFAULT_PLAYER_DATA_PATH;

	sweetLoader.GetOrCreate("PlayerDataPath") = playerData;

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

void PlayerController::HandleInput(float deltaTime)
{
	if (!RenderQueueSingleton::IsInitialized()) return;
	auto camera = RenderQueueSingleton::Get()->GetCameraController();

	if (m_bCameraOffsetDirty)
	{
		camera->SetOffsetToAttached(m_CameraOffset);
		m_bCameraOffsetDirty = false;
	}
}

void PlayerController::OnFocus()
{
	if (!RenderQueueSingleton::IsInitialized()) return;
	auto camera = RenderQueueSingleton::Get()->GetCameraController();

	camera->AttachCameraToObject(GetActorMesh());
	camera->LookAtAttached(true);
	camera->FollowAttached(true);
}

void PlayerController::OffFocus()
{
	if (!RenderQueueSingleton::IsInitialized()) return;
	auto camera = RenderQueueSingleton::Get()->GetCameraController();

	camera->AttachCameraToObject(nullptr);
	camera->LookAtAttached(false);
	camera->FollowAttached(false);
}

void PlayerController::SetCameraOffset(const DirectX::XMFLOAT3& offset)
{
	if (offset.x != m_CameraOffset.x ||
		offset.y != m_CameraOffset.y ||
		offset.z != m_CameraOffset.z)
	{
		m_CameraOffset = offset;
		m_bCameraOffsetDirty = true;
	}
}

DirectX::XMFLOAT3 PlayerController::GetCameraOffset() const
{
	return m_CameraOffset;
}
