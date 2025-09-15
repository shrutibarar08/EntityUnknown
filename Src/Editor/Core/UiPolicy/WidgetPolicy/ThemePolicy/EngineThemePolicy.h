#pragma once
#include <cstdint>
#include "imgui/imgui.h"

class LevelEditorContext;

struct EngineThemeConfig
{
    enum class Palette : uint8_t { Dark, Classic, Light, Dracula, SolarizedDark };

    // One-time setup
    bool     enableDocking = true;
    bool     enableViewports = false;     // ImGuiConfigFlags_ViewportsEnable (multi-viewport)
    Palette  palette = Palette::Dark;

    // Accent color (for buttons, headers, sliders)
    ImVec4   accent = ImVec4(0.18f, 0.52f, 0.95f, 1.0f); // nice blue

    // Per-frame scoped style vars (pushed in BeginTheme, popped in EndTheme)
    float    windowRounding = 6.0f;
    float    frameRounding = 5.0f;
    float    grabRounding = 5.0f;
    float    tabRounding = 5.0f;
    float    scrollbarRounding = 8.0f;

    float    alpha = 1.0f;     // global alpha tweak
    float    framePaddingX = 10.0f;
    float    framePaddingY = 6.0f;
};

class EngineThemePolicy
{
public:
    EngineThemePolicy() = default;

    bool Init(LevelEditorContext* context) { return true; }
    // entry points expected by your policy aggregator
    void BeginTheme(LevelEditorContext*);
    void EndTheme(LevelEditorContext*);

    // config access
    EngineThemeConfig& Config() { return m_cfg; }
    const EngineThemeConfig& Config() const { return m_cfg; }

private:
    // ensure one-time configuration (io flags + palette + accent mapping)
    void EnsureApplyOnce();
    void ApplyConfigFlags();
    void ApplyPalette();
    void ApplyAccent();

    // small scoped pushes/pops per frame
    void PushScopedStyle();
    void PopScopedStyle();

    // helper palettes
    void SetPalette_Dark();
    void SetPalette_Classic();
    void SetPalette_Light();
    void SetPalette_Dracula();
    void SetPalette_SolarizedDark();

private:
    EngineThemeConfig m_cfg{};
    bool              m_appliedOnce = false;
    int               m_pushedVars = 0;  // PopStyleVar count
};
