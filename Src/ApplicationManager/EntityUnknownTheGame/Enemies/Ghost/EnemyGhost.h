#pragma once
#include "ApplicationManager/EntityUnknownTheGame/Interface/IActor.h"
#include "RenderManager/Animation/SpriteAnimator/SpriteAnimStateMachine.h"


#define DEFAULT_Enemies_DATA_PATH "Data/Enemies/"


class EnemyGhost final: public virtual IActor
{
public:
	EnemyGhost();
	~EnemyGhost() override = default;

	void SetEnemyPath(const std::string& loadPath) { m_SaveLoadPath = loadPath; }

	EnemyGhost(const EnemyGhost&) = delete;
	EnemyGhost(EnemyGhost&&) = delete;
	EnemyGhost& operator=(const EnemyGhost&) = delete;
	EnemyGhost& operator=(EnemyGhost&&) = delete;

	void BuildCheck(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

	void OnBeginPlay(const SweetLoader& sweetData) override;
	void OnTick(float deltaTime) override;

	void SaveSweetData(SweetLoader& sweetLoader) override;
	IRender* GetActorMesh() const override;

	SpriteAnimStateMachine* GetAnimState() const override;

	bool IsNavigating() const { return m_bNavigating; }
	void SetNavigate(bool flag) { m_bNavigating = flag; }

private:
	void AIController();

public:
	void ActorSpecificBehaviourUI() override;

private:
	bool m_bNavigating{ false };
	std::string m_SaveLoadPath{};

	SweetLoader m_EnemyData{};
	std::unique_ptr<IRender> m_EnemyMesh{ nullptr };
	std::unique_ptr<SpriteAnimStateMachine> m_EnemyAnimation{ nullptr };

	DirectX::XMFLOAT3 m_LeftMostPosition{-30.f, 2.f, -4.9f};
	DirectX::XMFLOAT3 m_RightMostPosition{15.f, 2.f, -4.9f};
};
