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
	std::unique_ptr<ModelCube> m_Cube{ nullptr };
	std::unique_ptr<ModelCube> m_Cube_2{ nullptr };
	std::unique_ptr<DirectionalLight> m_Light{ nullptr };
	Timer m_Timer{};
	float x{ 0 };
};
