#include "IApplication.h"

#include "ExceptionManager/IException.h"
#include "SystemManager/EventQueue/EventQueue.h"

bool IApplication::Init()
{
	if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
	{
		LOG_ERROR("Failed To Max out process");
	}
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
	{
		LOG_ERROR("Failed To Set Thread Highest priority!");
	}

	m_WindowsSystem = std::make_unique<WindowsSystem>();
	m_PhysicsSystem = std::make_unique<PhysicsSystem>();
	m_RenderSystem = std::make_unique<RenderSystem>(m_WindowsSystem.get(), m_PhysicsSystem.get());
	m_RenderSystem->AttachSystemToRender(this);

	m_InputHandler = std::make_unique<InputHandler>(m_WindowsSystem.get());
	m_FreeController = std::make_unique<FreeController>();

	//~ Add all the ISystem classes to be initialized in correct order
	m_DependencyHandler.Register(m_WindowsSystem.get());
	m_DependencyHandler.Register(m_PhysicsSystem.get());
	m_DependencyHandler.Register(m_RenderSystem.get());
	m_DependencyHandler.Register(m_InputHandler.get());
	//~ Add Dependency so that it should initialize in structural order
	m_DependencyHandler.AddDependency(m_RenderSystem.get(), m_WindowsSystem.get(), m_PhysicsSystem.get());
	m_DependencyHandler.AddDependency(m_InputHandler.get(), m_WindowsSystem.get(), m_RenderSystem.get());

	//~ hehe
	m_DependencyHandler.InitAll(m_Config);

	if (!InitializeApplication(m_Config)) return false;

	//~ TODO: This is only for test later free camera should be attached to the level editor
	m_InputHandler->AddInputController(m_FreeController.get());
	m_InputHandler->FocusControlOn(m_FreeController->GetAssignedID());

	if (!m_RenderSystem->GetCameraController()) THROW("Render System giving null camera controller");
	m_FreeController->AttachCameraController(m_RenderSystem->GetCameraController());

	return true;
}

bool IApplication::Execute()
{
	m_Timer.Reset();

	while (true)
	{
		if (WindowsSystem::ProcessAndExit() || m_WindowsSystem->Keyboard.WasKeyPressed(VK_ESCAPE))
		{
			m_DependencyHandler.ShutdownAll(m_Config);
			m_Config.Save("ApplicationConfig.json"); // TODO: Make it Dynamic
			return true;
		}
		float deltaTime = m_Timer.Tick();
		m_NextFpsUpdate -= deltaTime;
		m_FrameCounts++;

		if (m_NextFpsUpdate <= 0.0f)
		{
			m_NextFpsUpdate = 1.0f;
			std::wstringstream ss;
			ss << L"FPS: " << m_FrameCounts;
			m_WindowsSystem->SetWindowName(ss.str());
			m_FrameCounts = 0;
		}

		if (!m_DependencyHandler.UpdateAllFrames(deltaTime)) LOG_ERROR("Failure in Main loop dependency handler!");
		EventBus::DispatchAll();
		if (!m_DependencyHandler.EndAllFrames()) LOG_ERROR("Failure in Main loop dependency handler!");
		Sleep(1);
	}
	return true;
}
