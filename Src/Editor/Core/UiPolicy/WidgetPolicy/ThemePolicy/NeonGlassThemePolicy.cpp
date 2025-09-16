#include "NeonGlassThemePolicy.h"
#include "imgui/imgui_internal.h"

#include "RenderManager/Components/ShaderResource/TextureResource/TextureLoader.h"

NeonGlassThemePolicy::NeonGlassThemePolicy() = default;

bool NeonGlassThemePolicy::Init(LevelEditorContext* context)
{
    ImGuiIO& io = ImGui::GetIO();
    bool ok = true;

    if (m_cfg.mainFontPath && m_cfg.mainFontPath[0] != '\0')
    {
        ImFontConfig cfg; cfg.OversampleH = 3; cfg.OversampleV = 1; cfg.PixelSnapH = false;
        ImFont* base = io.Fonts->AddFontFromFileTTF(m_cfg.mainFontPath, m_cfg.mainFontSize, &cfg);
        ok = ok && (base != nullptr);

        if (m_cfg.iconFontPath && m_cfg.iconFontPath[0] != '\0')
        {
            ImFontConfig iconCfg; iconCfg.MergeMode = true; iconCfg.PixelSnapH = true;
            io.Fonts->AddFontFromFileTTF(m_cfg.iconFontPath, m_cfg.iconFontSize, &iconCfg, nullptr);
        }
        ok = ok && io.Fonts->Build();
    }

    m_inited = true;
    return ok;
}

void NeonGlassThemePolicy::BeginTheme(LevelEditorContext* context)
{
    ApplyConfigFlags();
    ApplyStyleVars();
    ApplyPalette();
    ApplyAccent();

    ImGui::GetStyle().Alpha = m_cfg.globalAlpha;
}

void NeonGlassThemePolicy::EndTheme(LevelEditorContext* context)
{
    if (m_pushedColorVars > 0) { ImGui::PopStyleColor(m_pushedColorVars); m_pushedColorVars = 0; }
    if (m_pushedStyleVars > 0) { ImGui::PopStyleVar(m_pushedStyleVars);   m_pushedStyleVars = 0; }
}

NeonGlassThemeConfig& NeonGlassThemePolicy::Config() { return m_cfg; }
const NeonGlassThemeConfig& NeonGlassThemePolicy::Config() const { return m_cfg; }

void NeonGlassThemePolicy::ApplyConfigFlags()
{
    ImGuiIO& io = ImGui::GetIO();

    if (m_cfg.enableDocking)   io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    else                       io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;

    if (m_cfg.enableViewports) io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    else                       io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = m_cfg.windowRounding;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

void NeonGlassThemePolicy::ApplyStyleVars()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_cfg.windowRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_cfg.frameRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, m_cfg.grabRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, m_cfg.tabRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, m_cfg.scrollbarRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(m_cfg.framePaddingX, m_cfg.framePaddingY));
    m_pushedStyleVars += 6;
}

void NeonGlassThemePolicy::ApplyPalette()
{
    ImVec4* colors = ImGui::GetStyle().Colors;

    // Base dark
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.07f, 0.09f, m_cfg.glassiness);
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.06f, 0.08f, m_cfg.glassiness);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.25f, 0.6f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.12f, 0.16f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.20f, 0.26f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.22f, 0.28f, 1.0f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.07f, 0.8f);

    colors[ImGuiCol_MenuBarBg] = colors[ImGuiCol_TitleBg];
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.18f, 0.22f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.22f, 0.28f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.24f, 0.32f, 1.0f);

    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.18f, 0.22f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.22f, 0.28f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.24f, 0.32f, 1.0f);
}

void NeonGlassThemePolicy::ApplyAccent()
{
    ImVec4 accent = m_cfg.accent;
    ImVec4 accentHover = ImVec4(accent.x + 0.1f, accent.y + 0.1f, accent.z + 0.1f, 1.0f);
    ImVec4 accentActive = ImVec4(accent.x + 0.2f, accent.y + 0.2f, accent.z + 0.2f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_CheckMark, accent);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, accent);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, accentActive);
    ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, accentHover);
    ImGui::PushStyleColor(ImGuiCol_SeparatorActive, accentActive);
    ImGui::PushStyleColor(ImGuiCol_TabHovered, accentHover);
    ImGui::PushStyleColor(ImGuiCol_TabActive, accentActive);
    ImGui::PushStyleColor(ImGuiCol_ResizeGrip, accent);
    ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, accentHover);
    ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, accentActive);

    m_pushedColorVars += 10;
}
