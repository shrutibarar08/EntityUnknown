#pragma once
#include "ApplicationManager/IApplication.h"
#include "RenderManager/Model/ModelCube.h"
#include "Utils/Timer/Timer.h"


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
	std::vector<std::unique_ptr<IBitmap>> m_Bitmaps{};
	std::unique_ptr<IBitmap> m_Background{ nullptr };

	std::unique_ptr<ModelCube> m_Cube{ nullptr };
	std::unique_ptr<ModelCube> m_Cube_2{ nullptr };
	std::unique_ptr<ModelCube> m_Cube_3{ nullptr };
	std::unique_ptr<DirectionalLight> m_Light{ nullptr };
	std::unique_ptr<DirectionalLight> m_Light_2{ nullptr };

	bool m_Removed{ false };
	float m_WaitTime{ 5.f };
	Timer m_Timer{};
	float x{ 0 };

	float topLeft = -500.f;
	float padding = 230.f;
	float constantY = -250.3f;
};
