#include "WindowsSystem.h"

#include "EventQueue.h"
#include "ExceptionManager/WindowsException.h"

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

void WindowsSystem::SetFullScreen(bool flag)
{
    if (flag != m_FullScreen)
    {
        m_FullScreen = flag;
        m_FullScreen ? ApplyFullScreen() : ApplyWindowedScreen();
        UpdateWindow(GetWindowHandle());


        if (flag)
        {
            RECT rt; GetClientRect(GetWindowHandle(), &rt);
            FullScreenPayload payload{ rt.right - rt.left, rt.bottom - rt.top };
            EventBus::Enqueue(EventType::FullScreen, payload, 10);
        }else
        {
            RECT rt; GetClientRect(GetWindowHandle(), &rt);
            WindowedScreenPayload payload{ rt.right - rt.left, rt.bottom - rt.top };
            EventBus::Enqueue(EventType::WindowedScreen, payload, 10);
        }
    }
}

float WindowsSystem::GetAspectRatio() const
{
    return static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight);
}

const std::vector<RESOLUTION>& WindowsSystem::GetAvailableResolution() const
{
    return m_PossibleResolution;
}

void WindowsSystem::UpdateResolution(const RESOLUTION* resolution)
{
    if (!resolution || !m_WindowHandle)
        return;
    if (resolution->Height == m_WindowHeight && resolution->Width == m_WindowWidth) return;

    RECT desiredRect = { 0, 0, static_cast<LONG>(resolution->Width), static_cast<LONG>(resolution->Height) };
    AdjustWindowRect(&desiredRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = desiredRect.right - desiredRect.left;
    int windowHeight = desiredRect.bottom - desiredRect.top;

    // Step 2: Resize the window while keeping its current position
    SetWindowPos(
        m_WindowHandle,
        nullptr,
        0, 0,
        windowWidth,
        windowHeight,
        SWP_NOZORDER | SWP_NOMOVE
    );

    m_WindowWidth = resolution->Width;
    m_WindowHeight = resolution->Height;

    ApplyFullScreen();

    FullScreenPayload fullscreenData{ m_WindowWidth, m_WindowHeight };
    EventBus::Enqueue(EventType::FullScreen, fullscreenData, 2);
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
        THROW_WINDOWS_EXCEPTION();
        return false;
    }

    RECT rect = { 0, 0, static_cast<LONG>(m_WindowWidth), static_cast<LONG>(m_WindowHeight) };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE))
    {
        THROW_WINDOWS_EXCEPTION();
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
        THROW_WINDOWS_EXCEPTION();
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

void WindowsSystem::ApplyFullScreen()
{
    if (m_FullScreen)
    {
        GetWindowPlacement
        (
            GetWindowHandle(),
            &m_WindowPlacement
        );

        SetWindowLong(GetWindowHandle(), GWL_STYLE, WS_POPUP);
        SetWindowPos(
            GetWindowHandle(),
            HWND_TOP,
            0, 0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN),
            SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );
    }
}

void WindowsSystem::ApplyWindowedScreen() const
{
    if (!m_FullScreen)
    {
        SetWindowLong(GetWindowHandle(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(GetWindowHandle(), &m_WindowPlacement);
        SetWindowPos
        (
            GetWindowHandle(),
            nullptr,
            0, 0,
            0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
        );
    }
}
