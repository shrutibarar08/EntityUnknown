#include "WindowsSystem.h"

bool WindowsSystem::OnInit(const SweetLoader& sweetLoader)
{
    if (!InitWindows()) return false;
	return true;
}

bool WindowsSystem::OnTick(float deltaTime)
{
	return true;
}

bool WindowsSystem::OnExit(SweetLoader& sweetLoader)
{
	return true;
}

HWND WindowsSystem::GetWindowHandle() const
{
	return m_WindowHandle;
}

HINSTANCE WindowsSystem::GetWindowInstance() const
{
	return m_WindowInstance;
}

bool WindowsSystem::ProcessAndExit()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return true;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return false;
}

std::string WindowsSystem::GetSystemName()
{
    return "WindowsSystem";
}

bool WindowsSystem::InitWindows()
{
    m_WindowInstance = GetModuleHandle(nullptr);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProcSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG_PTR);
    wc.hInstance = m_WindowInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"MyWindowClass";
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, L"Failed to register window class!", L"Error", MB_ICONERROR);
        return false;
    }

    RECT rect = { 0, 0, m_WindowWidth, m_WindowHeight };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE))
    {
        MessageBox(nullptr, L"AdjustWindowRect failed!", L"Error", MB_ICONERROR);
        return false;
    }
    std::wstring wWindowName = std::wstring(m_WindowName.begin(), m_WindowName.end());

    int adjustedWidth = rect.right - rect.left;
    int adjustedHeight = rect.bottom - rect.top;

    m_WindowHandle = CreateWindowEx(
        0,
        wc.lpszClassName,
        wWindowName.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        adjustedWidth, adjustedHeight,
        nullptr,
        nullptr,
        m_WindowInstance,
        this);

    if (!m_WindowHandle)
    {
        MessageBox(nullptr, L"Failed to create window!", L"Error", MB_ICONERROR);
        return false;
    }

    ShowWindow(m_WindowHandle, SW_SHOW);
    UpdateWindow(m_WindowHandle);

    return true;
}

LRESULT WindowsSystem::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

LRESULT WindowsSystem::WindowProcSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCT* create = reinterpret_cast<CREATESTRUCT*>(lParam);
        WindowsSystem* that = reinterpret_cast<WindowsSystem*>(create->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowProcThunk));
        return that->MessageHandler(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT WindowsSystem::WindowProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WindowsSystem* that = reinterpret_cast<WindowsSystem*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (that)
    {
        return that->MessageHandler(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
