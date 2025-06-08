#include "MouseHandler.h"
#include <format>
#include "Utils/Logger/Logger.h"

MouseHandler::MouseHandler()
    : m_rawDeltaX(0), m_rawDeltaY(0), m_wheelDelta(0), m_cursorVisible(true)
{
    ZeroMemory(m_buttonDown, sizeof(m_buttonDown));
    ZeroMemory(m_buttonPressed, sizeof(m_buttonPressed));
    m_absPos = { 0,0 };
}

void MouseHandler::AttachHWnd(HWND hwnd)
{
    // Register for raw mouse input (generic desktop mouse).
    RAWINPUTDEVICE rid = { 0x01, 0x02, 0, hwnd };
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    m_hwnd = hwnd;
}

bool MouseHandler::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MOUSEMOVE:
    {
        m_absPos.x = GET_X_LPARAM(lParam);
        m_absPos.y = GET_Y_LPARAM(lParam);
        return true;
    }
    case WM_LBUTTONDOWN:
    {
        if (!m_buttonDown[0])
        {
            m_buttonPressed[0] = true;
            m_buttonDown[0] = true;
        }
        return true;
    }
    case WM_LBUTTONUP:
    {
        m_buttonDown[0] = false;
        return true;
    }
    case WM_RBUTTONDOWN:
    {
        if (!m_buttonDown[1])
        {
            m_buttonPressed[1] = true;
            m_buttonDown[1] = true;
        }
        return true;
    }
    case WM_RBUTTONUP:
    {
        m_buttonDown[1] = false;
        return true;
    }
    case WM_MBUTTONDOWN:
    {
        if (!m_buttonDown[2])
        {
            m_buttonPressed[2] = true;
            m_buttonDown[2] = true;
        }
        return true;
    }
    case WM_MBUTTONUP:
    {
        m_buttonDown[2] = false;
        return true;
    }
    case WM_MOUSEWHEEL:
    {
        // High-order word is wheel delta in multiples of WHEEL_DELTA:contentReference[oaicite:9]{index=9}
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        m_wheelDelta += delta;
        return true;
    }
    case WM_INPUT:
    {
        // Process raw input for mouse motion (based on MSDN example:contentReference[oaicite:10]{index=10}).
        UINT size = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
        if (size > 0)
        {
            std::vector<BYTE> data(size);
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, data.data(), &size, sizeof(RAWINPUTHEADER)) == size) {
                RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(data.data());
                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    m_rawDeltaX += raw->data.mouse.lLastX;
                    m_rawDeltaY += raw->data.mouse.lLastY;
                }
            }
        }
        return true;
    }
    }
    return false;
}

void MouseHandler::EndFrame()
{
    m_rawDeltaX = m_rawDeltaY = 0;
    m_wheelDelta = 0;
    ZeroMemory(m_buttonPressed, sizeof(m_buttonPressed));
}

void MouseHandler::LockCursorToWindow(bool center) const
{
    RECT rect;
    if (GetClientRect(m_hwnd, &rect))
    {
        POINT tl = { rect.left, rect.top }, br = { rect.right, rect.bottom };
        ClientToScreen(m_hwnd, &tl);
        ClientToScreen(m_hwnd, &br);
        RECT rt = RECT{ tl.x, tl.y, br.x, br.y };
        ClipCursor(&rt);
        if (center)
        {
            int cx = (tl.x + br.x) / 2, cy = (tl.y + br.y) / 2;
            SetCursorPos(cx, cy);
        }
    }
}

void MouseHandler::UnlockCursor() const
{
    ClipCursor(nullptr);
}
