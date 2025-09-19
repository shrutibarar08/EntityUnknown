#pragma once
#include <cstdint>
#include "imgui/imgui.h"

class LevelEditorContext;
struct ID3D11ShaderResourceView;

// ------------------------------------------------------------
// NeonGlass Theme (sexy & clean, dark w/ neon accents)
// ------------------------------------------------------------
struct NeonGlassThemeConfig
{
    enum class Palette : uint8_t { NeonDark, Midnight, Nord, TokyoNight };

    bool     enableDocking = true;
    bool     enableViewports = true;

    Palette  palette = Palette::NeonDark;
    ImVec4   accent = ImVec4(0.18f, 0.75f, 1.00f, 1.0f);
    float    globalAlpha = 1.0f;
    float    glassiness = 0.88f;
    float    shadowStrength = 0.35f;
    float    borderThickness = 1.0f;

    float    windowRounding = 8.0f;
    float    frameRounding = 6.0f;
    float    grabRounding = 6.0f;
    float    tabRounding = 7.0f;
    float    scrollbarRounding = 9.0f;

    float    framePaddingX = 12.0f;
    float    framePaddingY = 7.0f;

    const char* mainFontPath = "Assets/UI/font/AmaticSC-Bold.ttf";
    float       mainFontSize = 28.0f;
    const char* iconFontPath = "";
    float       iconFontSize = 24.0f;
};

class NeonGlassThemePolicy
{
public:
    NeonGlassThemePolicy();

    bool Init(LevelEditorContext* ctx);
    void BeginTheme(LevelEditorContext* ctx);
    void EndTheme(LevelEditorContext* ctx);

    NeonGlassThemeConfig& Config();
    const NeonGlassThemeConfig& Config() const;

private:
    void ApplyConfigFlags();
    void ApplyStyleVars();
    void ApplyPalette();
    void ApplyAccent();

private:
    NeonGlassThemeConfig m_cfg{};
    bool m_inited = false;
    int  m_pushedStyleVars = 0;
    int  m_pushedColorVars = 0;
};
