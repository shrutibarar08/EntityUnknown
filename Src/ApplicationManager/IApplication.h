#pragma once
#include <memory>

#include "RenderManager/RenderSystem.h"
#include "SystemManager/DependencyHandler/DependencyHandler.h"
#include "WindowsManager/WindowsSystem.h"
#include "ApplicationManager/InputHandler/FreeController/FreeController.h"
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
	bool Execute();

protected:
	virtual bool InitializeApplication(const SweetLoader& sweetLoader) = 0;
	virtual bool Update() = 0;

protected:
	Timer m_Timer{};
	SweetLoader m_Config{};
	DependencyHandler m_DependencyHandler{};
	std::unique_ptr<WindowsSystem> m_WindowsSystem{ nullptr };
	std::unique_ptr<RenderSystem> m_RenderSystem{ nullptr };
	std::unique_ptr<InputHandler> m_InputHandler{ nullptr };
	std::unique_ptr<FreeController> m_FreeController{ nullptr };
	std::unique_ptr<PhysicsSystem> m_PhysicsSystem{ nullptr };
};
