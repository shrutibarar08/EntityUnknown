#pragma once
#include "EntityPhysics.h"
#include "RenderManager/IRender.h"
#include "SystemManager/ISystem.h"


class PhysicsSystem final: public ISystem
{
public:
	PhysicsSystem() = default;
	~PhysicsSystem() override = default;

	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem(PhysicsSystem&&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(PhysicsSystem&&) = delete;

	bool OnInit(const SweetLoader& sweetLoader) override;
	bool OnFrameUpdate(float deltaTime) override;
	bool OnFrameEnd() override;
	bool OnExit(SweetLoader& sweetLoader) override;
	std::string GetSystemName() override;

	bool AddObject(IRender* renderObj);
	bool RemoveObject(const IRender* renderObj);
	bool RemoveObject(ID renderObjID);
	void Clear();

	void SetIntegration(IntegrationType type);

private:
	void Update(float deltaTime);

private:
	IntegrationType m_IntegrationType{ IntegrationType::SemiImplicitEuler };
	std::unordered_map<ID, IRender*> m_RenderedObjects{};
};
