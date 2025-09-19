#pragma once
#include <type_traits>
#include <utility>
#include <string>
#include "Imgui/imgui.h"

#include "WidgetPolicy/MainMenuPolicy/ImGuiMainMenuPolicy.h"
#include "WidgetPolicy/TabsPolicy/ImGuiPanelPolicy.h"
#include "WidgetPolicy/ContentBrowser/ImGuiContentBrowserPolicy.h"
#include "WidgetPolicy/CreationPolicy/ImguiCreationPanel.h"
#include "WidgetPolicy/CreationPolicy/ImguiCreationPanelHorizontal.h"

#include "WidgetPolicy/ThemePolicy/EngineThemePolicy.h"
#include "WidgetPolicy/ThemePolicy/NeonGlassThemePolicy.h"

#include "WidgetPolicy/CreationPolicy/CreationPanelConcepts.h"
#include "WidgetPolicy/InspectorPolicy/EngineInspectorPolicy.h"
#include "WidgetPolicy/HeaderPolicy/EngineHeaderPolicy.h"
#include "WidgetPolicy/ToolsPolicy/EngineToolPanelPolicy.h"

#include "Utils/Logger/Logger.h"

class LevelEditorContext;

// ---------- Core Concepts ----------
template<class T> concept HasBeginTheme = requires(T t, LevelEditorContext * c) { t.BeginTheme(c); };
template<class T> concept HasEndTheme = requires(T t, LevelEditorContext * c) { t.EndTheme(c); };
template<class T> concept HasBeginWorkspace = requires(T t, LevelEditorContext * c) { { t.BeginWorkspace(c) } -> std::convertible_to<bool>; };
template<class T> concept HasEndWorkspace = requires(T t, LevelEditorContext * c) { t.EndWorkspace(c); };
template<class T> concept HasEndPanel = requires(T t, LevelEditorContext * c) { t.EndPanel(c); };
template<class T> concept HasDrawHeader = requires(T t, LevelEditorContext * c) { t.DrawHeader(c); };
template<class T> concept HasDrawTools = requires(T t, LevelEditorContext * c) { t.DrawTools(c); };
template<class T> concept HasInit = requires(T t, LevelEditorContext * c) { { t.Init(c) } -> std::convertible_to<bool>; };
template<class T>
concept HasBeginPanel = requires(T t, LevelEditorContext * c) {
    { t.BeginPanel(c, "Title", PanelSlot::Left, ImGuiWindowFlags_None) } -> std::convertible_to<bool>;
};

template<
    class TContentBrowserPolicy,
    class TMainMenuPolicy,
    class TThemePolicy,
    class TCreationPolicy,
    class TInspectorPolicy,
    class TToolsPolicy,
    class THeaderPolicy,
    class TPanelPolicy
>
class ImGuiPolicy
{
    static_assert(HasCreationRegister<TCreationPolicy>, "TCreationPolicy must support Register(Item[/vector]).");
    static_assert(HasDrawCreation<TCreationPolicy>, "TCreationPolicy must implement DrawCreation(LevelEditorContext*).");
    static_assert(HasDrawHeader<THeaderPolicy>, "THeaderPolicy must implement DrawHeader(LevelEditorContext*).");
    static_assert(HasDrawTools <TToolsPolicy >, "TToolsPolicy must implement DrawTools(LevelEditorContext*).");
    static_assert(HasInit<TContentBrowserPolicy>, "TContentBrowserPolicy must implement bool Init(LevelEditorContext*).");
    static_assert(HasInit<TMainMenuPolicy>, "TMainMenuPolicy must implement bool Init(LevelEditorContext*).");
    static_assert(HasInit<TThemePolicy>, "TThemePolicy must implement bool Init(LevelEditorContext*).");
    static_assert(HasInit<TCreationPolicy>, "TCreationPolicy must implement bool Init(LevelEditorContext*).");
    static_assert(HasInit<TInspectorPolicy>, "TInspectorPolicy must implement bool Init(LevelEditorContext*).");
    static_assert(HasInit<THeaderPolicy>, "THeaderPolicy must implement bool Init(LevelEditorContext*).");
    static_assert(HasInit<TPanelPolicy>, "TPanelPolicy must implement bool Init(LevelEditorContext*).");

public:
    ImGuiPolicy()
    {}

    std::string name() const { return "ImGui"; }

    bool Init(LevelEditorContext* context)
    {
        if (!m_contentBrowser.Init(context))
        {
            LOG_ERROR("Failed to initialize Content Browser");
            return false;
        }
        if (!m_mainMenu.Init(context))
        {
            LOG_ERROR("Failed to initialize Main Menu");
            return false;
        }
        if (!m_theme.Init(context))
        {
            LOG_ERROR("Failed to initialize Theme");
            return false;
        }
        if (!m_create.Init(context))
        {
            LOG_ERROR("Failed to initialize Creation Panel");
            return false;
        }
        if (!m_inspector.Init(context))
        {
            LOG_ERROR("Failed to initialize Inspector");
            return false;
        }
        if (!m_tools.Init(context))
        {
            LOG_ERROR("Failed to initialize Tools");
            return false;
        }
        if (!m_header.Init(context))
        {
            LOG_ERROR("Failed to initialize Header");
            return false;
        }
        if (!m_panels.Init(context))
        {
            LOG_ERROR("Failed to initialize Panels");
            return false;
        }

        LOG_INFO("Initialized all ImGui UI policies successfully");
        return true;
    }

    void Render(LevelEditorContext* ctx)
    {
        BeginThemeIf(ctx);
        //m_mainMenu.DrawMainMenu(ctx);

        const bool workspace_open = BeginWorkspaceIf(ctx);
        if constexpr (HasBeginPanel<TPanelPolicy> && HasEndPanel<TPanelPolicy>)
            DrawPanels(ctx);
        else
            DrawFallbackWindows(ctx);

        EndWorkspaceIf(ctx, workspace_open);
        EndThemeIf(ctx);
    }

    void Update(LevelEditorContext* /*ctx*/, float /*dt*/) {}

    // Access to policies (for runtime config)
    TContentBrowserPolicy& Content() { return m_contentBrowser; }
    TMainMenuPolicy& Menu() { return m_mainMenu; }
    TThemePolicy& Theme() { return m_theme; }
    TCreationPolicy& CreateLevel() { return m_create; }
    TInspectorPolicy& Inspector() { return m_inspector; }
    TToolsPolicy& Tools() { return m_tools; }
    THeaderPolicy& Header() { return m_header; }
    TPanelPolicy& Panels() { return m_panels; }

private:
    
    void BeginThemeIf(LevelEditorContext* ctx) { if constexpr (HasBeginTheme<TThemePolicy>) m_theme.BeginTheme(ctx); }
    void EndThemeIf(LevelEditorContext* ctx) { if constexpr (HasEndTheme  <TThemePolicy>) m_theme.EndTheme(ctx); }

    bool BeginWorkspaceIf(LevelEditorContext* ctx)
    {
        if constexpr (HasBeginWorkspace<TPanelPolicy>) return m_panels.BeginWorkspace(ctx);
        return false;
    }
    void EndWorkspaceIf(LevelEditorContext* ctx, bool open)
    {
        if constexpr (HasEndWorkspace<TPanelPolicy>) if (open) m_panels.EndWorkspace(ctx);
    }

    // ---- Panels path ----
    void DrawPanels(LevelEditorContext* ctx)
    {
        DrawHeaderPanel(ctx);   // Top
        DrawInspectorPanel(ctx);// Left
        DrawToolsPanel(ctx);    // Right
        DrawCreatePanel(ctx);   // Center
        DrawAssetsPanel(ctx);   // Bottom
    }

    void DrawHeaderPanel(LevelEditorContext* ctx)
    {
        if (m_panels.BeginPanel(ctx, "Header", PanelSlot::Top,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
        {
            m_header.DrawHeader(ctx);
        }
        m_panels.EndPanel(ctx);
    }

    void DrawInspectorPanel(LevelEditorContext* ctx)
    {
        if (m_panels.BeginPanel(ctx, "Inspector", PanelSlot::Left, ImGuiWindowFlags_NoCollapse))
        {
            m_inspector.DrawInspector(ctx);
        }
        m_panels.EndPanel(ctx);
    }

    void DrawToolsPanel(LevelEditorContext* ctx)
    {
        if (m_panels.BeginPanel(ctx, "Tools", PanelSlot::Right, ImGuiWindowFlags_NoCollapse))
        {
            m_tools.DrawTools(ctx);
        }
        m_panels.EndPanel(ctx);
    }

    void DrawCreatePanel(LevelEditorContext* ctx)
    {
        if (m_panels.BeginPanel(ctx, "Create", PanelSlot::Center, ImGuiWindowFlags_NoCollapse))
        {
            m_create.DrawCreation(ctx);
        }
        m_panels.EndPanel(ctx);
    }

    void DrawAssetsPanel(LevelEditorContext* ctx)
    {
        if (m_panels.BeginPanel(ctx, "Assets", PanelSlot::Bottom, ImGuiWindowFlags_NoCollapse))
        {
            m_contentBrowser.DrawContentBrowser(ctx);
        }
        m_panels.EndPanel(ctx);
    }

    // Fallback
    void DrawFallbackWindows(LevelEditorContext* ctx)
    {
        DrawWindow("Header", [&] { m_header.DrawHeader(ctx);     }, ImGuiWindowFlags_NoTitleBar);
        DrawWindow("Inspector", [&] { m_inspector.DrawInspector(ctx); });
        DrawWindow("Tools", [&] { m_tools.DrawTools(ctx);       });
        DrawWindow("Create", [&] { m_create.DrawCreation(ctx);   });
        DrawWindow("Assets", [&] { m_contentBrowser.DrawContentBrowser(ctx); });
    }

    template<class Fn>
    void DrawWindow(const char* title, Fn body, ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse)
    {
        if (ImGui::Begin(title, nullptr, flags)) { body(); }
        ImGui::End();
    }

private:
    TContentBrowserPolicy m_contentBrowser;
    TMainMenuPolicy       m_mainMenu;
    TThemePolicy          m_theme;
    TCreationPolicy       m_create;
    TInspectorPolicy      m_inspector;
    TToolsPolicy          m_tools;
    THeaderPolicy         m_header;
    TPanelPolicy          m_panels;
};


using ImGuiPolicy_Default = ImGuiPolicy<
    ImGuiContentBrowserPolicy,
    ImGuiMainMenuPolicy,
    NeonGlassThemePolicy,
    ImguiCreationPanel,
    EngineInspectorPolicy,
    EngineToolPanelPolicy,
    EngineHeaderPolicy,
    ImGuiPanelPolicy
>;
