#pragma once
#include "Components/KeyboardHandler.h"
#include "Components/MouseHandler.h"
#include "Src/SystemManager/ISystem.h"

typedef struct RESOLUTION
{
	UINT Width;
	UINT Height;
}RESOLUTION;

class WindowsSystem final: public ISystem
{
public:
	WindowsSystem() = default;
	~WindowsSystem() override = default;

	WindowsSystem(const WindowsSystem&) = delete;
	WindowsSystem(WindowsSystem&&) = delete;

	WindowsSystem& operator=(const WindowsSystem&) = delete;
	WindowsSystem& operator=(WindowsSystem&&) = delete;

	bool OnInit(const SweetLoader& sweetLoader) override;
	bool OnFrameUpdate(float deltaTime) override;
	bool OnFrameEnd() override;
	bool OnExit(SweetLoader& sweetLoader) override;

	HWND GetWindowHandle() const;
	HINSTANCE GetWindowHInstance() const;

	static bool ProcessAndExit();
	std::string GetSystemName() override;

	KeyboardHandler Keyboard{};
	MouseHandler Mouse{};
  
	bool IsFullScreen() const { return m_FullScreen; }
	void SetFullScreen(bool flag);

	float GetAspectRatio() const;
	const std::vector<RESOLUTION>& GetAvailableResolution() const;
	void UpdateResolution(const RESOLUTION* resolution);

	int GetWindowsHeight() const { return m_WindowHeight; }
	int GetWindowsWidth() const { return m_WindowWidth; }

private:
	bool InitWindows();

	LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WindowProcSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WindowProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void ApplyFullScreen();
	void ApplyWindowedScreen() const;

private:
	std::string m_WindowName{ "Default Window Name" };
	std::string m_ClassName{ "WINDOW_SYSTEM_CLASS_MANAGER" };

	UINT m_WindowHeight{ 720 };
	UINT m_WindowWidth{ 1280 };

	HWND m_WindowHandle{ nullptr };
	HINSTANCE m_WindowInstance{ nullptr };

	WINDOWPLACEMENT m_WindowPlacement = { sizeof(m_WindowPlacement) };
	bool m_FullScreen{ false };
	std::vector<RESOLUTION> m_PossibleResolution = {
			{640, 480},      // VGA (legacy)
			{800, 600},      // SVGA
			{1024, 768},     // XGA (4:3)
			{1280, 720},     // HD / 720p
			{1366, 768},     // WXGA
			{1440, 900},     // WXGA+ (16:10)
			{1600, 900},     // HD+ (16:9)
			{1680, 1050},    // WSXGA+ (16:10)
			{1920, 1080},    // Full HD / 1080p
			{1920, 1200},    // WUXGA (16:10)
			{2048, 1152},    // QWXGA
			{2560, 1080},    // Ultrawide FHD (21:9)
			{2560, 1440},    // QHD / 1440p
			{2560, 1600},    // WQXGA (16:10)
			{2880, 1620},    // QHD+ upscale
			{3440, 1440},    // Ultrawide QHD
			{3840, 1600},    // UWQHD+
			{3840, 2160},    // 4K UHD
			{5120, 1440},    // Dual QHD / Super Ultrawide
			{5120, 2160},    // 5K Ultrawide
			{5760, 1080},    // Triple-monitor 1080p
			{7680, 4320},    // 8K UHD
	};
};
