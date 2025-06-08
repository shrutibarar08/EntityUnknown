#pragma once
#include <Windows.h>
#include <unordered_map>

class KeyboardHandler
{
public:
    KeyboardHandler();

    // Call from WndProc. Returns true if the message was handled.
    bool HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    // Call once per frame after processing input
    void EndFrame();

    // Query methods:
    bool IsKeyDown(int key) const;
    bool WasKeyPressed(int key) const;

private:
    static constexpr int KEY_COUNT = 256;
    bool m_currentKeyState[KEY_COUNT]; // true if key is currently down
    bool m_keyPressed[KEY_COUNT];      // true only on the first frame after keydown
};
