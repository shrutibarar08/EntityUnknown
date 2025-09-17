#include "KeyboardHandler.h"
#include <cstring>

KeyboardHandler::KeyboardHandler() noexcept
{
    std::memset(m_down, 0, sizeof(m_down));
    std::memset(m_pressed, 0, sizeof(m_pressed));
    std::memset(m_released, 0, sizeof(m_released));
}

bool KeyboardHandler::IsAutoRepeat(LPARAM lParam) noexcept
{
    return (lParam & (1 << 30)) != 0;
}

void KeyboardHandler::ClearAll() noexcept
{
    std::memset(m_down, 0, sizeof(m_down));
    std::memset(m_pressed, 0, sizeof(m_pressed));
    std::memset(m_released, 0, sizeof(m_released));
}

bool KeyboardHandler::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (msg)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        const int vk = static_cast<int>(wParam);
        if (!InRange(vk)) return false;

        if (!m_down[vk]) // key was up before
        {
            if (!IsAutoRepeat(lParam))
                m_pressed[vk] = true;
            m_down[vk] = true;
        }
        return true;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        const int vk = static_cast<int>(wParam);
        if (!InRange(vk)) return false;
        if (m_down[vk]) {
            m_released[vk] = true;
            m_down[vk] = false;
        }
        return true;
    }
    case WM_KILLFOCUS:
    case WM_SETFOCUS:
        ClearAll();
        return false;
    default:
        return false;
    }
}

void KeyboardHandler::EndFrame() noexcept
{
    std::memset(m_pressed, 0, sizeof(m_pressed));
    std::memset(m_released, 0, sizeof(m_released));
}

bool KeyboardHandler::IsKeyDown(int vk) const noexcept
{
    return InRange(vk) ? m_down[vk] : false;
}

bool KeyboardHandler::WasKeyPressed(int vk) const noexcept
{
    return InRange(vk) ? m_pressed[vk] : false;
}

bool KeyboardHandler::WasKeyReleased(int vk) const noexcept
{
    return InRange(vk) ? m_released[vk] : false;
}

bool KeyboardHandler::IsCtrlDown()  const noexcept { return m_down[VK_CONTROL] || m_down[VK_LCONTROL] || m_down[VK_RCONTROL]; }
bool KeyboardHandler::IsShiftDown() const noexcept { return m_down[VK_SHIFT] || m_down[VK_LSHIFT] || m_down[VK_RSHIFT]; }
bool KeyboardHandler::IsAltDown()   const noexcept { return m_down[VK_MENU] || m_down[VK_LMENU] || m_down[VK_RMENU]; }
bool KeyboardHandler::IsSuperDown() const noexcept { return m_down[VK_LWIN] || m_down[VK_RWIN]; }

bool KeyboardHandler::WasChordPressed(int mainKey, uint8_t mods) const noexcept
{
    if (!WasKeyPressed(mainKey)) return false;

    if ((mods & Ctrl) && !IsCtrlDown())  return false;
    if ((mods & Shift) && !IsShiftDown()) return false;
    if ((mods & Alt) && !IsAltDown())   return false;
    if ((mods & Super) && !IsSuperDown()) return false;

    return true;
}

bool KeyboardHandler::WasComboPressed(std::initializer_list<int> keys) const noexcept
{
    bool anyPressed = false;
    for (int vk : keys) {
        if (!IsKeyDown(vk)) return false;
        anyPressed = anyPressed || WasKeyPressed(vk);
    }
    return anyPressed;
}
