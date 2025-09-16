#include "IApplication.h"

#include "ExceptionManager/IException.h"
#include "SystemManager/EventQueue/EventQueue.h"

bool IApplication::Init()
{
	//~ Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
	{
		LOG_ERROR("Failed To Max out process");
	}
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
	{
		LOG_ERROR("Failed To Set Thread Highest priority!");
	}
	m_Config.Load(m_ConfigPath);
	m_WindowsSystem = std::make_unique<WindowsSystem>();
	m_PhysicsSystem = std::make_unique<PhysicsSystem>();
	m_RenderSystem = std::make_unique<RenderSystem>(m_WindowsSystem.get(), m_PhysicsSystem.get());
	m_InputHandler = std::make_unique<InputHandler>(m_WindowsSystem.get());
	m_Editor = std::make_unique<EditorUI_ImGui>();

#ifdef _DEBUG
	m_RenderSystem->AttachSystemToRender(m_Editor.get());
#endif

	//~ Add all the ISystem classes to be initialized in correct order
	m_DependencyHandler.Register(m_WindowsSystem.get());
	m_DependencyHandler.Register(m_PhysicsSystem.get());
	m_DependencyHandler.Register(m_RenderSystem.get());
	m_DependencyHandler.Register(m_InputHandler.get());
	m_DependencyHandler.Register(m_Editor.get());

	//~ Add Dependency so that it should initialize in structural order
	m_DependencyHandler.AddDependency(m_RenderSystem.get(), m_WindowsSystem.get(), m_PhysicsSystem.get());
	m_DependencyHandler.AddDependency(m_InputHandler.get(), m_WindowsSystem.get(), m_RenderSystem.get());
	m_DependencyHandler.AddDependency(m_Editor.get(), m_WindowsSystem.get(), m_RenderSystem.get());

	//~ hehe
	m_DependencyHandler.InitAll(m_Config);

	if (!m_RenderSystem->GetCameraController()) THROW("Render System giving null camera controller");

	return InitializeApplication(m_Config);
}

bool IApplication::GameLoop()
{
	m_Timer.Reset();

	while (true)
	{
		if (WindowsSystem::ProcessAndExit() || m_WindowsSystem->Keyboard.WasKeyPressed(VK_ESCAPE))
		{
			m_DependencyHandler.ShutdownAll(m_Config);
			SaveSweetData(m_Config);
			m_Config.Save(m_ConfigPath);
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
		Update(deltaTime);

		if (!m_DependencyHandler.CleanAllFrames()) LOG_ERROR("Failure in Main loop dependency handler!");
		Sleep(1);
	}
	return true;
}
