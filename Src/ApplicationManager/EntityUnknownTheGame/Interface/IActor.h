#pragma once
#include "IEntity.h"
#include "RenderManager/Animation/SpriteAnimator/SpriteAnimStateMachine.h"
#include "RenderManager/Sprite/WorldSpaceSprite/WorldSpaceSprite.h"


enum class ActorAnimState : uint8_t
{
	IDLE,
	WALKING_LEFT,
	WALKING_RIGHT,
	JUMPING,
	FALLING,
	FALLING_LEFT,
	FALLING_RIGHT,
	DEAD
};

class IActor : public IEntity
{
public:
	~IActor() override = default;
	virtual IRender* GetActorMesh() const = 0;

	virtual void BuildCheck(ID3D11Device* device, ID3D11DeviceContext* deviceContext) {};

	static const char* ToString(ActorAnimState state);
	static ActorAnimState PlayerAnimStateFromString(const std::string& str);

	virtual SpriteAnimStateMachine* GetAnimState() const = 0;

	float GetRunningSpeed() const { return m_RunningSpeed; }
	float GetMaxRunningSpeed() const { return m_MaxRunningVelocityX; }
	float GetJumpingForce() const { return m_JumpingForce; }

	void SetRunningSpeed(float value) { m_RunningSpeed = value; }
	void SetMaxRunningSpeed(float value) { m_MaxRunningVelocityX = value; }
	void SetJumpingForce(float value) { m_JumpingForce = value; }

	DirectX::XMFLOAT3 GetActorStartPosition() const { return m_ActorStartPosition; }
	void SetActorStartPosition(const DirectX::XMFLOAT3& pos) { m_ActorStartPosition = pos; }

	virtual void ActorSpecificBehaviourUI() {}

protected:

	float m_RunningSpeed{ 30.0f };
	float m_MaxRunningVelocityX{ 5.f };
	float m_JumpingForce{ 5.0f };

	DirectX::XMFLOAT3 m_ActorStartPosition{ 0, 10, 0 };
};

inline const char* IActor::ToString(ActorAnimState state)
{
	switch (state)
	{
	case ActorAnimState::IDLE:          return "IDLE";
	case ActorAnimState::WALKING_LEFT:  return "WALKING_LEFT";
	case ActorAnimState::WALKING_RIGHT: return "WALKING_RIGHT";
	case ActorAnimState::JUMPING:       return "JUMPING";
	case ActorAnimState::FALLING:       return "FALLING";
	case ActorAnimState::FALLING_LEFT:  return "FALLING_LEFT";
	case ActorAnimState::FALLING_RIGHT: return "FALLING_RIGHT";
	case ActorAnimState::DEAD:			return "DEAD";
	default:                             return "UNKNOWN";
	}
}

inline ActorAnimState IActor::PlayerAnimStateFromString(const std::string& str)
{
	if (str == "IDLE")          return ActorAnimState::IDLE;
	if (str == "WALKING_LEFT")  return ActorAnimState::WALKING_LEFT;
	if (str == "WALKING_RIGHT") return ActorAnimState::WALKING_RIGHT;
	if (str == "JUMPING")       return ActorAnimState::JUMPING;
	if (str == "FALLING")       return ActorAnimState::FALLING;
	if (str == "FALLING_LEFT")       return ActorAnimState::FALLING_LEFT;
	if (str == "FALLING_RIGHT")       return ActorAnimState::FALLING_RIGHT;
	if (str == "DEAD")       return ActorAnimState::DEAD;

	throw std::runtime_error("Invalid ActorAnimState string: " + str);
}
