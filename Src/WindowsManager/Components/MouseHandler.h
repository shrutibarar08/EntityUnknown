#pragma once
#include <cstdint>
#include <windows.h>
#include <vector>
#include <windowsx.h>

class MouseHandler
{
public:
    MouseHandler();

    void AttachHWnd(HWND hwnd);

    // Call from WndProc. Returns true if the message was handled.
    bool HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    // Call once per frame to clear deltas and one-shot flags.
    void EndFrame();

    // Query methods:
    void GetAbsolutePosition(int& x, int& y) const  { x = m_absPos.x; y = m_absPos.y; }
    void GetRawDelta(int& dx, int& dy) const        { dx = m_rawDeltaX; dy = m_rawDeltaY; }
    int  GetWheelDelta() const                      { return m_wheelDelta; }
    bool IsButtonDown(int i) const                  { return (i >= 0 && i < 3) ? m_buttonDown[i] : false; }
    bool WasButtonPressed(int i) const              { return (i >= 0 && i < 3) ? m_buttonPressed[i] : false; }
    void HideCursor()                               { if (m_cursorVisible) { ShowCursor(FALSE); m_cursorVisible = false; } }
    void UnHideCursor()                             { if (!m_cursorVisible) { ShowCursor(TRUE);  m_cursorVisible = true; } }

    void LockCursorToWindow(bool center) const;
    void UnlockCursor() const;

private:
    HWND m_hwnd{ nullptr };
    bool m_buttonDown[3];
    bool m_buttonPressed[3];
    POINT m_absPos;
    int m_rawDeltaX, m_rawDeltaY;
    int m_wheelDelta;
    bool m_cursorVisible;
};
