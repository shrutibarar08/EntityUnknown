#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <utility>
#include <algorithm>
#include <type_traits>
#include <cassert>

#include "Core/EditorContext.h"
#include "Core/Commands/Commands.h"
#include "Core/Policies.h"
#include "SystemManager/Registry/RegistryTool.h"

#include "SystemManager/ISystem.h"
#include "RenderManager/ISystemRender.h"

// ---------- Policy Concepts ----------
template<class S>
concept CStoragePolicy = requires(S s, LevelEditorContext * ctx, const std::string & path) {
    { s.LoadLevel(path, ctx) } -> std::same_as<bool>;
    { s.SaveLevel(path, ctx) } -> std::same_as<bool>;
};

template<class U>
concept CUIPolicy = requires(U u, LevelEditorContext * ctx) {
    { u.Menu(ctx) } -> std::same_as<void>;
};

template<class U>
concept CUndoPolicy = requires(U u) {
    { u.maxDepth } -> std::convertible_to<std::size_t>;
};

template<class H>
concept CHookPolicy = requires(H h) { true; };

template<
    CStoragePolicy TStoragePolicy = SweetLoaderStoragePolicy,
    CUIPolicy      TUIPolicy      = ImGuiPolicy,
    CUndoPolicy    TUndoPolicy    = BoundedUndoPolicy,
    CHookPolicy    TRenderHooks   = NoopRenderHookPolicy
>
class LevelEditor final : public ISystem, public ISystemRender
{
public:
    LevelEditor() noexcept
    {
        //~ Core
        m_pLevelManager = std::make_unique<LevelManager>();
        m_pCommandStack = std::make_unique<CommandStack>();
        m_pUndo = std::make_unique<TUndoPolicy>();

        // honor undo depth
        if constexpr (requires(CommandStack & cs, std::size_t n) { cs.SetMaxDepth(n); })
            m_pCommandStack->SetMaxDepth(m_pUndo->maxDepth);

        //~ Policies
        m_pStorage          = std::make_unique<TStoragePolicy>();
        m_pUserInteraction  = std::make_unique<TUIPolicy>     ();
        m_pRenderHooks      = std::make_unique<TRenderHooks>  ();

        //~ Editor Context (pointer-based)
        m_pEditorContext = std::make_unique<LevelEditorContext>(
            m_pLevelManager.get(),
            m_pCommandStack.get(),
            m_pStorage.get(),
            m_pUserInteraction.get()
        );
    }

    ~LevelEditor() override = default;

    LevelEditor(const LevelEditor&)             = delete;
    LevelEditor& operator=(const LevelEditor&)  = delete;

    LevelEditor(LevelEditor&&)                  = default;
    LevelEditor& operator=(LevelEditor&&)       = default;

    // ---------------- ISystem (Main loop, NO rendering) ----------------
    bool OnInit(const SweetLoader& sweetLoader) override
    {
        return true;
    }

    bool OnFrameUpdate(float deltaTime) override
    {
        if constexpr (requires(TUIPolicy & u, LevelEditorContext * c, float dt) { u.Update(c, dt); })
            m_pUserInteraction->Update(m_pEditorContext.get(), deltaTime);

        // Drive active tools
        for (auto& tool : m_ppActiveTools)
            if (tool) tool->Tick(m_pEditorContext.get());

        return true;
    }

    bool OnFrameClear() override { return true; }
    bool OnExit(SweetLoader&) override { return true; }
    std::string GetSystemName() override { return "LevelEditor"; }

    // ---------------- ISystemRender (Render loop) ----------------
    void RenderBegin() override
    {
        if constexpr (requires(TUIPolicy & u, LevelEditorContext * C) { u.BeginFrame(C); })
            m_pUserInteraction->BeginFrame(m_pEditorContext.get());
    }

    void RenderExecute() override
    {
        // Menu is required by concept
        m_pUserInteraction->Menu(m_pEditorContext.get());

        if constexpr (requires(TUIPolicy & u, LevelEditorContext * C) { u.DrawDockspace(C); })
            m_pUserInteraction->DrawDockspace(m_pEditorContext.get());
        if constexpr (requires(TUIPolicy & u, LevelEditorContext * C) { u.DrawInspector(C); })
            m_pUserInteraction->DrawInspector(m_pEditorContext.get());
    }

    void RenderEnd() override
    {
        if constexpr (requires(TUIPolicy & u, LevelEditorContext * C) { u.EndFrame(C); })
            m_pUserInteraction->EndFrame(m_pEditorContext.get());
    }

    LevelEditorContext* Context() noexcept { return m_pEditorContext.get(); }

private:
    std::unique_ptr<LevelManager>      m_pLevelManager{};
    std::unique_ptr<CommandStack>      m_pCommandStack{};
    std::unique_ptr<TStoragePolicy>    m_pStorage{};
    std::unique_ptr<TUIPolicy>         m_pUserInteraction{};
    std::unique_ptr<TUndoPolicy>       m_pUndo{};
    std::unique_ptr<TRenderHooks>      m_pRenderHooks{};

    std::unique_ptr<LevelEditorContext> m_pEditorContext{};

    std::vector<std::unique_ptr<ITool>> m_ppActiveTools{};
    std::vector<std::uint64_t>          m_selection{};
    std::uint64_t                       m_lastCreatedId{};
};

using LevelEditor_ImGui = LevelEditor<
    SweetLoaderStoragePolicy,
    ImGuiPolicy,
    BoundedUndoPolicy,
    NoopRenderHookPolicy
>;
