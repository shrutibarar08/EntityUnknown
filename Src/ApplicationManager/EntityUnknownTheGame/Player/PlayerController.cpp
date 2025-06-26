#include "PlayerController.h"

#include "RenderManager/RenderQueue/RenderQueue.h"


PlayerController::PlayerController()
{
	m_PlayerMesh = std::make_unique<WorldSpaceSprite>();
	m_PlayerMesh->SetTransparent(true);
	m_PlayerMesh->GetShaderResource()->SetTexture("Texture/idle/0_Reaper_Man_Idle_001.tga");
}

void PlayerController::OnBeginPlay(const SweetLoader& sweetData)
{
	std::string playerData = sweetData["PlayerDataPath"].GetValue();
	if (playerData.empty()) playerData = DEFAULT_PLAYER_DATA_PATH;
	m_PlayerData.Load(playerData);
	LoadInputControls(m_PlayerData);

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

	SaveInputControls();
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

	if (!m_KeyboardHandler || !m_PlayerMesh) return;

	auto* rigidBody = m_PlayerMesh->GetRigidBody();
	if (!rigidBody) return;

	// === Movement Force ===
	DirectX::XMVECTOR moveForce = DirectX::XMVectorZero();

	if (m_KeyboardHandler->IsKeyDown('A'))
		moveForce = DirectX::XMVectorAdd(moveForce, DirectX::XMVectorSet(-m_RunningSpeed, 0.0f, 0.0f, 0.0f));

	if (m_KeyboardHandler->IsKeyDown('D'))
		moveForce = DirectX::XMVectorAdd(moveForce, DirectX::XMVectorSet(m_RunningSpeed, 0.0f, 0.0f, 0.0f));

	// Apply lateral force
	rigidBody->AddForce(moveForce);

	if (m_KeyboardHandler->WasKeyPressed(VK_SPACE))
	{
		if (rigidBody->IsGrounded())
		{
			rigidBody->ApplyLinearImpulse(DirectX::XMVectorSet(0.0f, m_JumpingForce, 0.0f, 0.0f));
			rigidBody->SetGrounded(false);
			LOG_INFO("Jumped!");
		}
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

void PlayerController::LoadInputControls(const SweetLoader& sweetData)
{
	const auto& inputNode = sweetData["Input"];
	const auto& camOffset = inputNode["CameraOffset"];

	m_CameraOffset.x = camOffset["X"].AsFloat();
	m_CameraOffset.y = camOffset["Y"].AsFloat();
	m_CameraOffset.z = camOffset["Z"].AsFloat();

	m_RunningSpeed = inputNode["RunningSpeed"].AsFloat();
	m_JumpingForce = inputNode["JumpingForce"].AsFloat();
}

void PlayerController::SaveInputControls()
{
	auto& inputNode = m_PlayerData.GetOrCreate("Input");

	auto& camOffset = inputNode.GetOrCreate("CameraOffset");
	camOffset.GetOrCreate("X") = std::to_string(m_CameraOffset.x);
	camOffset.GetOrCreate("Y") = std::to_string(m_CameraOffset.y);
	camOffset.GetOrCreate("Z") = std::to_string(m_CameraOffset.z);

	inputNode.GetOrCreate("RunningSpeed") = std::to_string(m_RunningSpeed);
	inputNode.GetOrCreate("JumpingForce") = std::to_string(m_JumpingForce);
}
