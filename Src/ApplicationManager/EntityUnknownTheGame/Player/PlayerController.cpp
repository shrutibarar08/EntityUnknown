#include "PlayerController.h"

#include "RenderManager/RenderQueue/RenderQueue.h"


PlayerController::PlayerController()
{
	m_PlayerMesh = std::make_unique<WorldSpaceSprite>();
	m_PlayerMesh->SetTransparent(true);
	m_PlayerMesh->GetShaderResource()->SetTexture("Texture/idle/0_Reaper_Man_Idle_001.tga");

	m_PlayerAnimation = std::make_unique<SpriteAnimStateMachine>(static_cast<ISprite*>(m_PlayerMesh.get()));
	m_PlayerAnimation->AddState(ToString(ActorAnimState::IDLE));
	m_PlayerAnimation->AddState(ToString(ActorAnimState::WALKING_LEFT));
	m_PlayerAnimation->AddState(ToString(ActorAnimState::WALKING_RIGHT));
	m_PlayerAnimation->AddState(ToString(ActorAnimState::JUMPING));
	m_PlayerAnimation->AddState(ToString(ActorAnimState::FALLING));
	m_PlayerAnimation->AddState(ToString(ActorAnimState::FALLING_LEFT));
	m_PlayerAnimation->AddState(ToString(ActorAnimState::FALLING_RIGHT));
	m_PlayerAnimation->AddState(ToString(ActorAnimState::DEAD));
}

void PlayerController::BuildCheck(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	if (m_PlayerAnimation != nullptr) m_PlayerAnimation->Build(device, deviceContext);
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

	if (m_PlayerAnimation)
	{
		m_PlayerAnimation->LoadFromSweetData(m_PlayerData["AnimState"]);
	}

	m_PlayerData.GetOrCreate("PlayerDataPath") = playerData;
}

void PlayerController::OnTick(float deltaTime)
{
	if (m_PlayerAnimation != nullptr)
	{
		m_PlayerAnimation->Update(deltaTime);
	}
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

	if (m_PlayerAnimation)
	{
		m_PlayerData.GetOrCreate("AnimState") = m_PlayerAnimation->GetSweetData();
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

void PlayerController::PlayerInput(float deltaTime)
{
	if (!m_KeyboardHandler || !m_PlayerMesh || !m_PlayerAnimation) return;

	auto* rigidBody = m_PlayerMesh->GetRigidBody();
	if (!rigidBody) return;

	if (IsPlayerDead() && m_KeyboardHandler->WasKeyPressed(VK_SPACE))
	{
		PlayerLifeReset();
	}

	// === DEAD State ===
	if (IsPlayerDead())
	{
		m_PlayerAnimation->TransitionTo("DEAD");
		return;
	}

	// === Input State ===
	bool isPressingA = m_KeyboardHandler->IsKeyDown('A');
	bool isPressingD = m_KeyboardHandler->IsKeyDown('D');
	bool isPressingJump = m_KeyboardHandler->WasKeyPressed(VK_SPACE);

	// === Apply Horizontal Movement ===
	DirectX::XMVECTOR moveForce = DirectX::XMVectorZero();

	if (isPressingA)
		moveForce = DirectX::XMVectorSet(-m_RunningSpeed, 0.0f, 0.0f, 0.0f);
	else if (isPressingD)
		moveForce = DirectX::XMVectorSet(m_RunningSpeed, 0.0f, 0.0f, 0.0f);

	rigidBody->AddForce(moveForce);

	// Clamp X Velocity
	DirectX::XMVECTOR velocity = rigidBody->GetVelocity();
	float xVel = DirectX::XMVectorGetX(velocity);
	float yVel = DirectX::XMVectorGetY(velocity);

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
	if (isPressingJump && rigidBody->IsGrounded())
	{
		rigidBody->ApplyLinearImpulse(DirectX::XMVectorSet(0.0f, m_JumpingForce, 0.0f, 0.0f));
		rigidBody->SetGrounded(false);

		// Determine jump direction on impulse
		if (xVel < -0.1f)
			m_PlayerAnimation->TransitionTo("JUMPING_LEFT");
		else if (xVel > 0.1f)
			m_PlayerAnimation->TransitionTo("JUMPING_RIGHT");
		else
			m_PlayerAnimation->TransitionTo("JUMPING");

		return;
	}

	// === Animation State Logic ===
	const bool isGrounded = rigidBody->IsGrounded();

	if (!isGrounded)
	{
		if (yVel > 0.1f) // Jumping upward
		{
			if (xVel < -0.1f)
				m_PlayerAnimation->TransitionTo("JUMPING");
			else if (xVel > 0.1f)
				m_PlayerAnimation->TransitionTo("JUMPING");
			else
				m_PlayerAnimation->TransitionTo("JUMPING");
		}
		else // Falling downward
		{
			if (xVel < -0.1f)
				m_PlayerAnimation->TransitionTo("FALLING_LEFT");
			else if (xVel > 0.1f)
				m_PlayerAnimation->TransitionTo("FALLING_RIGHT");
			else
				m_PlayerAnimation->TransitionTo("FALLING");
		}
	}
	else
	{
		if (isPressingA)
			m_PlayerAnimation->TransitionTo("WALKING_LEFT");
		else if (isPressingD)
			m_PlayerAnimation->TransitionTo("WALKING_RIGHT");
		else
			m_PlayerAnimation->TransitionTo("IDLE");
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

SpriteAnimStateMachine* PlayerController::GetAnimState() const
{
	if (m_PlayerAnimation) return m_PlayerAnimation.get();
	return nullptr;
}

void PlayerController::HurtPlayer(int hurtValue)
{
	m_HealthBar -= hurtValue;

	if (m_HealthBar <= 0)
	{
		m_PlayerMesh->GetRigidBody()->SetVelocity({ 0.f, 0.f, 0.f });
	}
}
