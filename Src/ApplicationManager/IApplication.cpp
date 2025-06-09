#include "IApplication.h"

#include "SystemManager/EventQueue/EventQueue.h"

bool IApplication::Init()
{
	m_WindowsSystem = std::make_unique<WindowsSystem>();
	m_RenderSystem = std::make_unique<RenderSystem>(m_WindowsSystem.get());

	//~ Add all the ISystem classes to be initialized in correct order
	m_DependencyHandler.Register(m_WindowsSystem.get());
	m_DependencyHandler.Register(m_RenderSystem.get());

	//~ Add Dependency so that it should initialize in structural order
	m_DependencyHandler.AddDependency(m_RenderSystem.get(), m_WindowsSystem.get());

	//~ hehe
	m_DependencyHandler.InitAll(m_Config);

	if (!InitializeApplication(m_Config)) return false;

	return true;
}

bool IApplication::Execute()
{
	while (true)
	{
		if (WindowsSystem::ProcessAndExit() || m_WindowsSystem->Keyboard.WasKeyPressed(VK_ESCAPE))
		{
			m_DependencyHandler.ShutdownAll(m_Config);
			m_Config.Save("ApplicationConfig.json"); // TODO: Make it Dynamic
			return true;
		}
		m_DependencyHandler.RunAll(0.0f);
		Update();
		EventBus::DispatchAll();
	}

	return true;
}
