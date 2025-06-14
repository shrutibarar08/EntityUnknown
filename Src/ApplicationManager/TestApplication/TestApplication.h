#pragma once
#include "ApplicationManager/IApplication.h"
#include "RenderManager/Model/ModelCube.h"
#include "Utils/Timer/Timer.h"

#include <memory>

#include "RenderManager/Animation/SpriteAnimator/SpriteAnim.h"

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
	bool Update() override;

private:
	std::vector<std::unique_ptr<ScreenSprite>> m_HeartSprite{};
	std::unique_ptr<BackgroundSprite> m_Background{ nullptr };

	std::unique_ptr<WorldSpaceSprite> m_AnimationSpriteHolder{ nullptr };
	std::unique_ptr<SpriteAnim> m_SpriteAnim{ nullptr };

	std::unique_ptr<ModelCube> m_Ground{ nullptr };
	std::unique_ptr<DirectionalLight> m_Light{ nullptr };

	std::vector<std::unique_ptr<WorldSpaceSprite>> m_Clouds{};
	std::vector<std::unique_ptr<ModelCube>> m_Cubes{};

	bool m_Removed{ false };
	float m_WaitTime{ 5.f };
	Timer m_Timer{};
	float x{ 0 };

	float topLeft = -500.f;
	float padding = 230.f;
	float constantY = -250.3f;
};
