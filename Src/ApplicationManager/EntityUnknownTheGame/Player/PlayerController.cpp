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
		m_PlayerMesh->GetRigidBody()->SetVelocity({ 0.0f, 0.0f, 0.0f });
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
	CameraInput(deltaTime);
	PlayerInput(deltaTime);
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

	m_MaxRunningVelocityX = inputNode["m_MaxRunningVelocityX"].AsFloat();
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
	inputNode.GetOrCreate("m_MaxRunningVelocityX") = std::to_string(m_MaxRunningVelocityX);
	inputNode.GetOrCreate("JumpingForce") = std::to_string(m_JumpingForce);
}

void PlayerController::PlayerInput(float deltaTime) const
{
	if (!m_KeyboardHandler || !m_PlayerMesh) return;

	auto* rigidBody = m_PlayerMesh->GetRigidBody();
	if (!rigidBody) return;

	// === Movement Force ===
	DirectX::XMVECTOR moveForce = DirectX::XMVectorZero();

	if (m_KeyboardHandler->IsKeyDown('A'))
		moveForce = DirectX::XMVectorAdd(moveForce, DirectX::XMVectorSet(-m_RunningSpeed, 0.0f, 0.0f, 0.0f));

	if (m_KeyboardHandler->IsKeyDown('D'))
		moveForce = DirectX::XMVectorAdd(moveForce, DirectX::XMVectorSet(m_RunningSpeed, 0.0f, 0.0f, 0.0f));

	rigidBody->AddForce(moveForce);

	// === Clamp X Velocity ===
	DirectX::XMVECTOR velocity = rigidBody->GetVelocity();
	float xVel = DirectX::XMVectorGetX(velocity);

	if (xVel > m_MaxRunningVelocityX)
	{
		velocity = DirectX::XMVectorSetX(velocity, m_MaxRunningVelocityX);
		rigidBody->SetVelocity(velocity);
	}
	else if (xVel < -m_MaxRunningVelocityX)
	{
		velocity = DirectX::XMVectorSetX(velocity, -m_MaxRunningVelocityX);
		rigidBody->SetVelocity(velocity);
	}

	// === Jump ===
	if (m_KeyboardHandler->WasKeyPressed(VK_SPACE))
	{
		if (rigidBody->IsGrounded())
		{
			rigidBody->ApplyLinearImpulse(DirectX::XMVectorSet(0.0f, m_JumpingForce, 0.0f, 0.0f));
			rigidBody->SetGrounded(false);
		}
	}
}

void PlayerController::CameraInput(float deltaTime)
{
	if (!RenderQueueSingleton::IsInitialized()) return;

	auto camera = RenderQueueSingleton::Get()->GetCameraController();
	if (!m_KeyboardHandler) return;

	const float zoomSpeed = 10.0f;    // Z adjustment
	const float panSpeed = 5.0f;      // X/Y adjustment

	// === Zoom (Z axis) ===
	if (m_KeyboardHandler->IsKeyDown(VK_OEM_PLUS) || m_KeyboardHandler->IsKeyDown(VK_ADD))
	{
		m_CameraOffset.z -= zoomSpeed * deltaTime;
		m_bCameraOffsetDirty = true;
	}
	if (m_KeyboardHandler->IsKeyDown(VK_OEM_MINUS) || m_KeyboardHandler->IsKeyDown(VK_SUBTRACT))
	{
		m_CameraOffset.z += zoomSpeed * deltaTime;
		m_bCameraOffsetDirty = true;
	}

	// === Pan (X/Y axis) ===
	if (m_KeyboardHandler->IsKeyDown(VK_LEFT))
	{
		m_CameraOffset.x -= panSpeed * deltaTime;
		m_bCameraOffsetDirty = true;
	}
	if (m_KeyboardHandler->IsKeyDown(VK_RIGHT))
	{
		m_CameraOffset.x += panSpeed * deltaTime;
		m_bCameraOffsetDirty = true;
	}
	if (m_KeyboardHandler->IsKeyDown(VK_UP))
	{
		m_CameraOffset.y += panSpeed * deltaTime;
		m_bCameraOffsetDirty = true;
	}
	if (m_KeyboardHandler->IsKeyDown(VK_DOWN))
	{
		m_CameraOffset.y -= panSpeed * deltaTime;
		m_bCameraOffsetDirty = true;
	}

	// === Apply updated offset ===
	if (m_bCameraOffsetDirty)
	{
		camera->SetOffsetToAttached(m_CameraOffset);
		m_bCameraOffsetDirty = false;
	}
}
