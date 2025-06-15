#pragma once
#include "ApplicationManager/IApplication.h"
#include "RenderManager/Model/Cube/ModelCube.h"
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

	std::unique_ptr<ScreenSprite> m_HelloSprite{ nullptr };
	std::unique_ptr<WorldSpaceSprite> BirdSprite_1{ nullptr };
	std::unique_ptr<WorldSpaceSprite> BirdSprite_2{ nullptr };
	std::unique_ptr<WorldSpaceSprite> BirdSprite_3{ nullptr };
	std::unique_ptr<SpriteAnim> m_BirdSpriteAnim_1{ nullptr };
	std::unique_ptr<SpriteAnim> m_BirdSpriteAnim_2{ nullptr };
	std::unique_ptr<SpriteAnim> m_BirdSpriteAnim_3{ nullptr };

	std::unique_ptr<ModelCube> m_Ground{ nullptr };
	std::unique_ptr<DirectionalLight> m_Light{ nullptr };

	std::vector<std::unique_ptr<WorldSpaceSprite>> m_Clouds{};

	std::unique_ptr<ModelCube> m_Left{ nullptr };
	std::unique_ptr<ModelCube> m_Right{ nullptr };

	bool m_Removed{ false };
	float m_WaitTime{ 5.f };
	Timer m_Timer{};
	float x{ 0 };

	float topLeft = -500.f;
	float padding = 230.f;
	float constantY = -250.3f;
};
