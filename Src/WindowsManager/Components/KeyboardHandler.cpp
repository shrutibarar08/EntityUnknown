#include "KeyboardHandler.h"

KeyboardHandler::KeyboardHandler()
{
    ZeroMemory(m_currentKeyState, sizeof(m_currentKeyState));
    ZeroMemory(m_keyPressed, sizeof(m_keyPressed));
}

bool KeyboardHandler::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
    {
        int key = (int)wParam;
        // On first key-down, mark "pressed once" and down
        if (!m_currentKeyState[key]) {
            m_keyPressed[key] = true;
            m_currentKeyState[key] = true;
        }
        return true;
    }
    if (msg == WM_KEYUP || msg == WM_SYSKEYUP) {
        int key = (int)wParam;
        m_currentKeyState[key] = false;
        return true;
    }
    return false;
}

void KeyboardHandler::EndFrame()
{
    ZeroMemory(m_keyPressed, sizeof(m_keyPressed)); // clear one-shot flags
}

bool KeyboardHandler::IsKeyDown(int key) const
{
    return m_currentKeyState[key];
}

bool KeyboardHandler::WasKeyPressed(int key) const
{
    return m_keyPressed[key];
}
