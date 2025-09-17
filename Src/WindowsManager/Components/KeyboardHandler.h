#pragma once
#define NOMINMAX
#include <Windows.h>
#include <initializer_list>
#include <cstdint>
#include <algorithm>

class KeyboardHandler
{
public:
    enum Mod : uint8_t 
    {
        None = 0,
        Ctrl = 1 << 0,
        Shift = 1 << 1,
        Alt = 1 << 2,
        Super = 1 << 3
    };

public:
    KeyboardHandler() noexcept;

    bool HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

    void EndFrame() noexcept;

    bool IsKeyDown(int vk)         const noexcept;
    bool WasKeyPressed(int vk)     const noexcept;
    bool WasKeyReleased(int vk)    const noexcept;

    bool WasChordPressed(int mainKey, uint8_t mods = None) const noexcept;
    bool WasComboPressed(std::initializer_list<int> keys) const noexcept;

private:
    static constexpr int KEY_COUNT = 256;
    bool m_down[KEY_COUNT];
    bool m_pressed[KEY_COUNT];
    bool m_released[KEY_COUNT];

    static inline bool InRange(int vk) noexcept { return vk >= 0 && vk < KEY_COUNT; }
    static bool IsAutoRepeat(LPARAM lParam) noexcept;
    void ClearAll() noexcept;

    // Helpers for modifiers
    bool IsCtrlDown()  const noexcept;
    bool IsShiftDown() const noexcept;
    bool IsAltDown()   const noexcept;
    bool IsSuperDown() const noexcept;
};
