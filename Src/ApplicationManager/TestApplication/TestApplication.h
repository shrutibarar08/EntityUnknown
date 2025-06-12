#pragma once
#include "ApplicationManager/IApplication.h"
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
	std::unique_ptr<IBitmap> m_MainMenu{ nullptr };

	std::unique_ptr<ModelCube> m_Cube{ nullptr };
	std::unique_ptr<ModelCube> m_Cube_2{ nullptr };
	std::unique_ptr<ModelCube> m_Cube_3{ nullptr };
	std::unique_ptr<DirectionalLight> m_Light{ nullptr };
	std::unique_ptr<DirectionalLight> m_Light_2{ nullptr };
	bool m_Removed{ false };
	float m_WaitTime{ 5.f };
	Timer m_Timer{};
	float x{ 0 };

	float topLeft = -8.1f;
	float padding = 1.1f;
	float constantY = 5.3f;
};

