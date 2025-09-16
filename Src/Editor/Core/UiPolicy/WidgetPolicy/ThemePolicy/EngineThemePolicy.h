#pragma once
#include <cstdint>
#include "imgui/imgui.h"

class LevelEditorContext;

struct EngineThemeConfig
{
    enum class Palette : uint8_t { Dark, Classic, Light, Dracula, SolarizedDark };

    bool     enableDocking      = true;
    bool     enableViewports    = true;
    Palette  palette            = EngineThemeConfig::Palette::Dracula;
    ImVec4   accent             = ImVec4(224.f / 256.f, 139.f / 256.f, 190.f / 256.f, 1.0f);

    float    windowRounding     = 6.0f;
    float    frameRounding      = 5.0f;
    float    grabRounding       = 5.0f;
    float    tabRounding        = 5.0f;
    float    scrollbarRounding  = 8.0f;

    float    alpha          = 1.0f;
    float    framePaddingX  = 10.0f;
    float    framePaddingY  = 6.0f;
};

class EngineThemePolicy
{
public:
    EngineThemePolicy() = default;

    bool Init(LevelEditorContext* context) { return true; }
    void BeginTheme(LevelEditorContext*);
    void EndTheme(LevelEditorContext*);

    EngineThemeConfig& Config() { return m_cfg; }
    const EngineThemeConfig& Config() const { return m_cfg; }

private:
    void EnsureApplyOnce();
    void ApplyConfigFlags();
    void ApplyPalette();
    void ApplyAccent();

    void PushScopedStyle();
    void PopScopedStyle();

    void SetPalette_Dark();
    void SetPalette_Classic();
    void SetPalette_Light();
    void SetPalette_Dracula();
    void SetPalette_SolarizedDark();

private:
    EngineThemeConfig m_cfg{};
    bool              m_appliedOnce = false;
    int               m_pushedVars = 0;
};
