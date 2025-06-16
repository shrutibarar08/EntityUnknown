#pragma once
#include "ApplicationManager/IApplication.h"
#include "RenderManager/Model/Cube/ModelCube.h"
#include "Utils/Timer/Timer.h"
#include <memory>

class TestApplication final: public IApplication
{
public:
	TestApplication() = default;
	~TestApplication() override = default;

	TestApplication(const TestApplication&) = delete;
	TestApplication(TestApplication&&) = delete;
	TestApplication& operator=(const TestApplication&) = delete;
	TestApplication& operator=(TestApplication&&) = delete;

protected:
	bool InitializeApplication(const SweetLoader& sweetLoader) override;
	void Update() override;

public:
	void RenderBegin() override;
	void RenderExecute() override;
	void RenderEnd() override;

private:
	void SpotLightControl();
	void DirectionalLightControl();
	void PointLightControl();
	void BackgroundControl();

private:
	std::unique_ptr<ModelCube> m_Ground{ nullptr };
	std::unique_ptr<DirectionalLight> m_DirectionalLight{ nullptr };
	std::unique_ptr<SpotLight> m_SpotLight{ nullptr };
	std::unique_ptr<PointLight> m_PointLight{ nullptr };

	std::unique_ptr<ModelCube> m_Left{ nullptr };
	std::unique_ptr<ModelCube> m_Right{ nullptr };

	//~ 2D Space Sprite
	std::unique_ptr<WorldSpaceSprite> m_Bird{ nullptr };
	std::unique_ptr<BackgroundSprite> m_Background{ nullptr };

	bool m_Removed{ false };
	float m_WaitTime{ 5.f };
	Timer m_Timer{};
	float x{ 0 };

	float m_SpotLightUpdate{ 30.0f };
};
