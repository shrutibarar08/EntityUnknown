#include "EnemyGhost.h"

#include "ApplicationManager/EntityUnknownTheGame/Player/PlayerController.h"
#include "Imgui/imgui.h"
#include "RenderManager/RenderQueue/RenderQueue.h"

EnemyGhost::EnemyGhost()
{
	m_EnemyMesh = std::make_unique<WorldSpaceSprite>();
	m_EnemyMesh->GetCubeCollider()->SetColliderState(ColliderState::Trigger);
	m_EnemyMesh->SetTransparent(true);
	m_EnemyMesh->GetShaderResource()->SetTexture("Texture/idle/0_Reaper_Man_Idle_001.tga");

	m_EnemyAnimation = std::make_unique<SpriteAnimStateMachine>(static_cast<ISprite*>(m_EnemyMesh.get()));
	m_EnemyAnimation->AddState(ToString(ActorAnimState::IDLE));
	m_EnemyAnimation->AddState(ToString(ActorAnimState::WALKING_LEFT));
	m_EnemyAnimation->AddState(ToString(ActorAnimState::WALKING_RIGHT));
	m_EnemyAnimation->AddState(ToString(ActorAnimState::DEAD));
	m_EnemyAnimation->AddState(ToString(ActorAnimState::FALLING_LEFT));
	m_EnemyAnimation->AddState(ToString(ActorAnimState::FALLING_RIGHT));

	m_EnemyAnimation->SetInitialState(ToString(ActorAnimState::WALKING_LEFT));
}

void EnemyGhost::BuildCheck(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	if (m_EnemyAnimation != nullptr) m_EnemyAnimation->Build(device, deviceContext);
}

void EnemyGhost::OnBeginPlay(const SweetLoader& sweetData)
{
	std::string path = DEFAULT_Enemies_DATA_PATH + m_SaveLoadPath;
	m_EnemyData.Load(path);

	const auto& inputNode = m_EnemyData["Input"];
	m_MaxRunningVelocityX = inputNode["m_MaxRunningVelocityX"].AsFloat();
	m_RunningSpeed = inputNode["RunningSpeed"].AsFloat();

	if (inputNode.Contains("StartPosition"))
	{
		const SweetLoader& startPosition = inputNode["StartPosition"];
		m_ActorStartPosition.x = startPosition["x"].AsFloat();
		m_ActorStartPosition.y = startPosition["y"].AsFloat();
		m_ActorStartPosition.z = startPosition["z"].AsFloat();
	}

	if (inputNode.Contains("LeftPosition"))
	{
		const SweetLoader& leftPosition = inputNode["LeftPosition"];
		m_LeftMostPosition.x = leftPosition["x"].AsFloat();
		m_LeftMostPosition.y = leftPosition["y"].AsFloat();
		m_LeftMostPosition.z = leftPosition["z"].AsFloat();
	}

	if (inputNode.Contains("RightPosition"))
	{
		const SweetLoader& rightPosition = inputNode["RightPosition"];
		m_RightMostPosition.x = rightPosition["x"].AsFloat();
		m_RightMostPosition.y = rightPosition["y"].AsFloat();
		m_RightMostPosition.z = rightPosition["z"].AsFloat();
	}

	if (m_EnemyMesh)
	{
		m_EnemyMesh->SetSweetData(m_EnemyData.GetOrCreate("RigidBody"));
		m_EnemyMesh->GetRigidBody()->SetVelocity({ 0.0f, 0.0f, 0.0f });
		if (RenderQueueSingleton::IsInitialized())
		{
			RenderQueueSingleton::Get()->AddRender(m_EnemyMesh.get());
		}
	}

	if (m_EnemyAnimation)
	{
		m_EnemyAnimation->LoadFromSweetData(m_EnemyData["AnimState"]);
	}

	m_EnemyData.GetOrCreate("DataPath") = path;
}

void EnemyGhost::OnTick(float deltaTime)
{
	IActor::OnTick(deltaTime);

	if (m_EnemyAnimation != nullptr)
	{
		m_EnemyAnimation->Update(deltaTime);
		AIController();
	}
}

void EnemyGhost::SaveSweetData(SweetLoader& sweetLoader)
{

	std::string playerData = m_EnemyData["DataPath"].GetValue();
	if (playerData.empty()) playerData = DEFAULT_Enemies_DATA_PATH + m_SaveLoadPath;

	sweetLoader.GetOrCreate("DataPath") = playerData;

	if (m_EnemyMesh)
	{
		m_EnemyData.GetOrCreate("RigidBody") = m_EnemyMesh->GetSweetData();
	}

	if (m_EnemyAnimation)
	{
		m_EnemyData.GetOrCreate("AnimState") = m_EnemyAnimation->GetSweetData();
	}

	auto& inputNode = m_EnemyData.GetOrCreate("Input");

	inputNode.GetOrCreate("RunningSpeed") = std::to_string(m_RunningSpeed);
	inputNode.GetOrCreate("m_MaxRunningVelocityX") = std::to_string(m_MaxRunningVelocityX);

	inputNode.GetOrCreate("StartPosition").GetOrCreate("x") = std::to_string(m_ActorStartPosition.x);
	inputNode.GetOrCreate("StartPosition").GetOrCreate("y") = std::to_string(m_ActorStartPosition.y);
	inputNode.GetOrCreate("StartPosition").GetOrCreate("z") = std::to_string(m_ActorStartPosition.z);

	inputNode.GetOrCreate("LeftPosition").GetOrCreate("x") = std::to_string(m_LeftMostPosition.x);
	inputNode.GetOrCreate("LeftPosition").GetOrCreate("y") = std::to_string(m_LeftMostPosition.y);
	inputNode.GetOrCreate("LeftPosition").GetOrCreate("z") = std::to_string(m_LeftMostPosition.z);

	inputNode.GetOrCreate("RightPosition").GetOrCreate("x") = std::to_string(m_RightMostPosition.x);
	inputNode.GetOrCreate("RightPosition").GetOrCreate("y") = std::to_string(m_RightMostPosition.y);
	inputNode.GetOrCreate("RightPosition").GetOrCreate("z") = std::to_string(m_RightMostPosition.z);

	m_EnemyData.Save(playerData);
}

IRender* EnemyGhost::GetActorMesh() const
{
	if (m_EnemyMesh) return m_EnemyMesh.get();
	return nullptr;
}

SpriteAnimStateMachine* EnemyGhost::GetAnimState() const
{
	if (m_EnemyAnimation) return m_EnemyAnimation.get();
	return nullptr;
}

void EnemyGhost::AIController()
{
	if (!m_EnemyMesh) return;

	auto* rigidBody = m_EnemyMesh->GetRigidBody();
	if (!rigidBody) return;

	if (!m_bNavigating)
	{
		rigidBody->SetVelocity({ 0, 0, 0 });
		return;
	}

	const float epsilon = 0.1f;

	// Get current position
	DirectX::XMVECTOR currentPos = rigidBody->GetPosition();
	DirectX::XMFLOAT3 current;
	DirectX::XMStoreFloat3(&current, currentPos);

	// Convert target to XMVECTOR
	DirectX::XMVECTOR target = DirectX::XMVectorZero();

	if (m_EnemyAnimation->IsInState(ToString(ActorAnimState::WALKING_LEFT)))
	{
		target = DirectX::XMLoadFloat3(&m_LeftMostPosition);
	}
	else if (m_EnemyAnimation->IsInState(ToString(ActorAnimState::WALKING_RIGHT)))
	{
		target = DirectX::XMLoadFloat3(&m_RightMostPosition);
	}

	// Direction vector
	DirectX::XMVECTOR direction = DirectX::XMVectorSubtract(target, currentPos);
	direction = DirectX::XMVector3Normalize(direction);

	// Set velocity or apply force
	rigidBody->SetVelocity(DirectX::XMVectorScale(direction, GetRunningSpeed()));

	// Switch state if close enough
	DirectX::XMVECTOR distanceVec = DirectX::XMVectorSubtract(target, currentPos);
	float distSqr = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(distanceVec));

	if (distSqr < epsilon * epsilon)
	{
		// Toggle direction
		if (m_EnemyAnimation->IsInState(ToString(ActorAnimState::WALKING_LEFT)))
		{
			m_EnemyAnimation->TransitionTo(ToString(ActorAnimState::WALKING_RIGHT));
		}
		else
		{
			m_EnemyAnimation->TransitionTo(ToString(ActorAnimState::WALKING_LEFT));
		}
	}
}

void EnemyGhost::ActorSpecificBehaviourUI()
{
	if (ImGui::CollapsingHeader("Ghost Patrol Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Set patrol endpoints:");

		// === Left Most ===
		float leftPos[3] = { m_LeftMostPosition.x, m_LeftMostPosition.y, m_LeftMostPosition.z };
		if (ImGui::DragFloat3("Left Most", leftPos, 0.1f))
		{
			m_LeftMostPosition = { leftPos[0], leftPos[1], leftPos[2] };
		}

		// === Right Most ===
		float rightPos[3] = { m_RightMostPosition.x, m_RightMostPosition.y, m_RightMostPosition.z };
		if (ImGui::DragFloat3("Right Most", rightPos, 0.1f))
		{
			m_RightMostPosition = { rightPos[0], rightPos[1], rightPos[2] };
		}

		// === Navigate Checkbox ===
		ImGui::Checkbox("Navigate", &m_bNavigating);
	}
}
