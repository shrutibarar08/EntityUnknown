#pragma once
#include <memory>

#include "RenderManager/RenderSystem.h"
#include "SystemManager/DependencyHandler/DependencyHandler.h"
#include "WindowsManager/WindowsSystem.h"
#include "LevelEditorManager/LevelEditor.h"
#include "PhysicsManager/PhysicsSystem.h"
#include "Utils/Timer/Timer.h"


class IApplication
{
public:
	IApplication() = default;
	virtual ~IApplication() = default;

	IApplication(const IApplication&) = delete;
	IApplication(IApplication&&) = delete;
	IApplication& operator=(const IApplication&) = delete;
	IApplication& operator=(IApplication&&) = delete;

	bool Init();
	bool GameLoop();

protected:
	virtual bool InitializeApplication(const SweetLoader& sweetLoader) { return true; }
	virtual void Update(float deltaTime){}
	virtual void SaveSweetData(SweetLoader& sweetLoader){}

protected:
	Timer m_Timer{};
	SweetLoader m_Config{};
	DependencyHandler m_DependencyHandler{};
	std::unique_ptr<WindowsSystem> m_WindowsSystem{ nullptr };
	std::unique_ptr<RenderSystem> m_RenderSystem{ nullptr };
	std::unique_ptr<PhysicsSystem> m_PhysicsSystem{ nullptr };
	std::unique_ptr<LevelEditor> m_LevelEditor{ nullptr };
	std::unique_ptr<InputHandler> m_InputHandler{ nullptr };

	float m_NextFpsUpdate{ 1.0f };
	int m_FrameCounts{ 0 };

private:
	const std::string m_ConfigPath{ "ApplicationConfig.json" };
};
