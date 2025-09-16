#include "EngineThemePolicy.h"
#include "imgui/imgui_internal.h"


void EngineThemePolicy::BeginTheme(LevelEditorContext*)
{
    EnsureApplyOnce();
    PushScopedStyle();
}

void EngineThemePolicy::EndTheme(LevelEditorContext*)
{
    PopScopedStyle();
}

void EngineThemePolicy::EnsureApplyOnce()
{
    if (m_appliedOnce) return;
    ApplyConfigFlags();
    ApplyPalette();
    ApplyAccent();
    m_appliedOnce = true;
}

void EngineThemePolicy::ApplyConfigFlags()
{
    ImGuiIO& io = ImGui::GetIO();
    if (m_cfg.enableDocking)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    else
        io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;

    if (m_cfg.enableViewports)
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    else
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
}

void EngineThemePolicy::ApplyPalette()
{
    switch (m_cfg.palette)
    {
    case EngineThemeConfig::Palette::Dark:           SetPalette_Dark(); break;
    case EngineThemeConfig::Palette::Classic:        SetPalette_Classic(); break;
    case EngineThemeConfig::Palette::Light:          SetPalette_Light(); break;
    case EngineThemeConfig::Palette::Dracula:        SetPalette_Dracula(); break;
    case EngineThemeConfig::Palette::SolarizedDark:  SetPalette_SolarizedDark(); break;
    }
}

void EngineThemePolicy::ApplyAccent()
{
    ImGuiStyle& style = ImGui::GetStyle();
    const ImVec4 a = m_cfg.accent;
    const ImVec4 aH = ImVec4(a.x, a.y, a.z, 0.86f);
    const ImVec4 aA = ImVec4(a.x, a.y, a.z, 1.00f);

    style.Colors[ImGuiCol_Button] = a;
    style.Colors[ImGuiCol_ButtonHovered] = aH;
    style.Colors[ImGuiCol_ButtonActive] = aA;

    style.Colors[ImGuiCol_SliderGrab] = a;
    style.Colors[ImGuiCol_SliderGrabActive] = aA;

    style.Colors[ImGuiCol_CheckMark] = aA;

    style.Colors[ImGuiCol_Header] = ImVec4(a.x, a.y, a.z, 0.35f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(a.x, a.y, a.z, 0.55f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(a.x, a.y, a.z, 0.75f);

    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(a.x, a.y, a.z, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive] = aA;

    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(a.x, a.y, a.z, 0.30f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(a.x, a.y, a.z, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = aA;

    style.Colors[ImGuiCol_TabHovered] = ImVec4(a.x, a.y, a.z, 0.65f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(a.x, a.y, a.z, 0.85f);
}

void EngineThemePolicy::PushScopedStyle()
{
    m_pushedVars = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_cfg.windowRounding);   ++m_pushedVars;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_cfg.frameRounding);    ++m_pushedVars;
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, m_cfg.grabRounding);     ++m_pushedVars;
    ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, m_cfg.tabRounding);      ++m_pushedVars;
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, m_cfg.scrollbarRounding); ++m_pushedVars;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_cfg.alpha);            ++m_pushedVars;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(m_cfg.framePaddingX, m_cfg.framePaddingY)); ++m_pushedVars;
}

void EngineThemePolicy::PopScopedStyle()
{
    if (m_pushedVars > 0)
        ImGui::PopStyleVar(m_pushedVars);
    m_pushedVars = 0;
}

void EngineThemePolicy::SetPalette_Dark()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowMenuButtonPosition = ImGuiDir_Right;
    s.Colors[ImGuiCol_WindowBg].w = 1.0f;
    s.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.94f);
    s.Colors[ImGuiCol_Border] = ImVec4(0.26f, 0.26f, 0.28f, 0.40f);
}

void EngineThemePolicy::SetPalette_Classic()
{
    ImGui::StyleColorsClassic();
    ImGuiStyle& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.13f, 1.0f);
    s.Colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.11f, 0.98f);
}

void EngineThemePolicy::SetPalette_Light()
{
    ImGui::StyleColorsLight();
    ImGuiStyle& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    s.Colors[ImGuiCol_Border] = ImVec4(0.35f, 0.35f, 0.38f, 0.40f);
}

void EngineThemePolicy::SetPalette_Dracula()
{
    ImGuiStyle& s = ImGui::GetStyle();
    s = ImGuiStyle();
    s.WindowPadding = ImVec2(8, 8);
    s.FramePadding = ImVec2(6, 4);
    s.ItemSpacing = ImVec2(8, 6);

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text]                = ImVec4(0.86f, 0.86f, 0.87f, 1.00f);
    c[ImGuiCol_TextDisabled]        = ImVec4(0.50f, 0.50f, 0.52f, 1.00f);
    c[ImGuiCol_WindowBg]            = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    c[ImGuiCol_ChildBg]             = ImVec4(0.10f, 0.10f, 0.13f, 1.00f);
    c[ImGuiCol_PopupBg]             = ImVec4(0.11f, 0.11f, 0.15f, 0.98f);
    c[ImGuiCol_Border]              = ImVec4(0.18f, 0.18f, 0.23f, 0.60f);
    c[ImGuiCol_FrameBg]             = ImVec4(0.20f, 0.20f, 0.26f, 1.00f);
    c[ImGuiCol_FrameBgHovered]      = ImVec4(0.26f, 0.26f, 0.33f, 1.00f);
    c[ImGuiCol_FrameBgActive]       = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
    c[ImGuiCol_TitleBg]             = ImVec4(0.16f, 0.16f, 0.22f, 1.00f);
    c[ImGuiCol_TitleBgActive]       = ImVec4(0.20f, 0.20f, 0.27f, 1.00f);
    c[ImGuiCol_MenuBarBg]           = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    c[ImGuiCol_ScrollbarBg]         = ImVec4(0.10f, 0.10f, 0.14f, 1.00f);
    c[ImGuiCol_ScrollbarGrab]       = ImVec4(0.34f, 0.34f, 0.44f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered]= ImVec4(0.42f, 0.42f, 0.54f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.52f, 0.52f, 0.64f, 1.00f);
    c[ImGuiCol_Tab]                 = ImVec4(0.20f, 0.20f, 0.28f, 1.00f);
    c[ImGuiCol_TabHovered]          = ImVec4(0.26f, 0.26f, 0.36f, 1.00f);
    c[ImGuiCol_TabActive]           = ImVec4(0.28f, 0.28f, 0.38f, 1.00f);
    c[ImGuiCol_Separator]           = ImVec4(0.24f, 0.24f, 0.32f, 1.00f);
}

void EngineThemePolicy::SetPalette_SolarizedDark()
{
    ImGuiStyle& s   = ImGui::GetStyle();
    s               = ImGuiStyle();
    ImVec4 base03   = ImVec4(0.00f, 0.17f, 0.21f, 1.00f);
    ImVec4 base02   = ImVec4(0.00f, 0.20f, 0.25f, 1.00f);
    ImVec4 base01   = ImVec4(0.39f, 0.54f, 0.52f, 1.00f);
    ImVec4 base0    = ImVec4(0.51f, 0.58f, 0.59f, 1.00f);

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text]            = base0;
    c[ImGuiCol_TextDisabled]    = ImVec4(base01.x, base01.y, base01.z, 1.0f);
    c[ImGuiCol_WindowBg]        = base03;
    c[ImGuiCol_ChildBg]         = base02;
    c[ImGuiCol_PopupBg]         = ImVec4(base02.x, base02.y, base02.z, 0.98f);
    c[ImGuiCol_Border]          = ImVec4(0.14f, 0.29f, 0.30f, 0.60f);
    c[ImGuiCol_FrameBg]         = ImVec4(0.09f, 0.26f, 0.30f, 1.00f);
    c[ImGuiCol_FrameBgHovered]  = ImVec4(0.12f, 0.34f, 0.38f, 1.00f);
    c[ImGuiCol_FrameBgActive]   = ImVec4(0.15f, 0.41f, 0.45f, 1.00f);
    c[ImGuiCol_TitleBg]         = ImVec4(0.03f, 0.20f, 0.23f, 1.00f);
    c[ImGuiCol_TitleBgActive]   = ImVec4(0.05f, 0.25f, 0.29f, 1.00f);
    c[ImGuiCol_Tab]             = ImVec4(0.07f, 0.27f, 0.31f, 1.00f);
    c[ImGuiCol_TabActive]       = ImVec4(0.11f, 0.33f, 0.37f, 1.00f);
    c[ImGuiCol_Separator]       = ImVec4(0.10f, 0.30f, 0.33f, 1.00f);
}
