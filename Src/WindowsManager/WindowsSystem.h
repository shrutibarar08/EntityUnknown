#pragma once
#include "Src/SystemManager/ISystem.h"


class WindowsSystem final: public ISystem
{
public:
	WindowsSystem() = default;
	virtual ~WindowsSystem() override = default;

	WindowsSystem(const WindowsSystem&) = delete;
	WindowsSystem(WindowsSystem&&) = delete;

	WindowsSystem& operator=(const WindowsSystem&) = delete;
	WindowsSystem& operator=(WindowsSystem&&) = delete;

	bool OnInit(const SweetLoader& sweetLoader) override;
	bool OnTick(float deltaTime) override;
	bool OnExit(SweetLoader& sweetLoader) override;

	HWND GetWindowHandle() const;
	HINSTANCE GetWindowInstance() const;

	static bool ProcessAndExit();
	std::string GetSystemName() override;

private:
	bool InitWindows();

	LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WindowProcSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WindowProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	std::string m_WindowName{ "Default Window Name" };
	std::string m_ClassName{ "WINDOW_SYSTEM_CLASS_MANAGER" };

	int m_WindowHeight{ 720 };
	int m_WindowWidth{ 1280 };

	HWND m_WindowHandle{ nullptr };
	HINSTANCE m_WindowInstance{ nullptr };
};
