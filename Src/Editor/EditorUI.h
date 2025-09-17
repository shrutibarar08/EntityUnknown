// EditorUI.h
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
#include "Core/InputContext/EditorInputHandler.h"

#include "SystemManager/ISystem.h"
#include "RenderManager/ISystemRender.h"

#include "nlohmann/json.hpp"

// ---------- Policy Concepts ----------
template<class S>
concept CStoragePolicy = requires(S s, LevelEditorContext * ctx)
{
    { s.Save(ctx) } -> std::same_as<bool>;
    { s.Load(ctx) } -> std::same_as<bool>;
};

template<class U>
concept CUIPolicy = requires(U u, LevelEditorContext * ctx)
{
    { u.Render(ctx) } -> std::same_as<void>;
    { u.Init(ctx) } -> std::same_as<bool>;
};

template<class U>
concept CUndoPolicy = requires(U u)
{
    { u.maxDepth } -> std::convertible_to<std::size_t>;
};

template<class H>
concept CHookPolicy = requires(H h) { true; };

template
<
    CStoragePolicy TStoragePolicy   = EditorStorage,
    CUIPolicy      TUIPolicy        = ImGuiPolicy_Default,
    CUndoPolicy    TUndoPolicy      = BoundedUndoPolicy,
    CHookPolicy    TRenderHooks     = NoopRenderHookPolicy
>
class EditorUI final : public ISystem, public ISystemRender
{
public:
    EditorUI() noexcept
    {
        //~ Core
        m_pLevelManager = std::make_unique<LevelManager>();
        m_pCommandStack = std::make_unique<CommandStack>();
        m_pUndo = std::make_unique<TUndoPolicy>();

        if constexpr (requires(CommandStack & cs, std::size_t n) { cs.SetMaxDepth(n); })
            m_pCommandStack->SetMaxDepth(m_pUndo->maxDepth);

        //~ Policies
        m_pStorage = std::make_unique<TStoragePolicy>();
        m_pUserInteraction = std::make_unique<TUIPolicy>();
        m_pRenderHooks = std::make_unique<TRenderHooks>();

        //~ Editor Context (pointer-based)
        m_pEditorContext = std::make_unique<LevelEditorContext>
            (
                m_pLevelManager.get(),
                m_pCommandStack.get(),
                m_pStorage.get()
            );

        m_inputHandler = std::make_unique<EditorInputHandler>();
    }

    ~EditorUI() override = default;

    EditorUI(const EditorUI&) = delete;
    EditorUI& operator=(const EditorUI&) = delete;
    EditorUI(EditorUI&&) = default;
    EditorUI& operator=(EditorUI&&) = default;

    // ---------------- ISystem (Main loop, NO rendering) ----------------
    bool OnInit(const SweetLoader&) override
    {
        m_pStorage->Load(m_pEditorContext.get());
        m_pUserInteraction->Init(m_pEditorContext.get());
        return true;
    }

    bool OnFrameUpdate(float deltaTime) override
    {
        m_inputHandler->HandleEditorInputs(m_pEditorContext.get());

        if constexpr (requires(TUIPolicy & u, LevelEditorContext * c, float dt) { u.Update(c, dt); })
            m_pUserInteraction->Update(m_pEditorContext.get(), deltaTime);

        for (auto& tool : m_ppActiveTools)
            if (tool) tool->Tick(m_pEditorContext.get());

        static float timePassed = 0.0f;
        timePassed += deltaTime;
        if (timePassed >= 0.5f && !m_pLevelManager->IsAnyLevelActive())
        {
            LOG_INFO("No Level Is Active");
            timePassed = 0.0f;
        }
        else if (timePassed >= 0.5f && m_pLevelManager->IsAnyLevelActive())
        {
            LOG_INFO("Active Level Found: " + m_pLevelManager->GetActiveLevelName());
            timePassed = 0.0f;
        }

        return true;
    }

    bool OnFrameClear() override { return true; }

    bool OnExit(SweetLoader&) override
    {
        m_pStorage->Save(m_pEditorContext.get());
        return true;
    }

    std::string GetSystemName() override { return "LevelEditor"; }

    // ---------------- ISystemRender (Render loop) ----------------
    void RenderBegin() override
    {
        if constexpr (requires(TUIPolicy & u, LevelEditorContext * C) { u.BeginFrame(C); })
            m_pUserInteraction->BeginFrame(m_pEditorContext.get());

        if constexpr (requires(TRenderHooks & h, LevelEditorContext * C) { h.OnBeginFrame(C); })
            m_pRenderHooks->OnBeginFrame(m_pEditorContext.get());
    }

    void RenderExecute() override
    {
        m_pUserInteraction->Render(m_pEditorContext.get());

        if constexpr (requires(TUIPolicy & u, LevelEditorContext * C) { u.DrawDockspace(C); })
            m_pUserInteraction->DrawDockspace(m_pEditorContext.get());
        if constexpr (requires(TUIPolicy & u, LevelEditorContext * C) { u.DrawInspector(C); })
            m_pUserInteraction->DrawInspector(m_pEditorContext.get());

        if constexpr (requires(TRenderHooks & h, LevelEditorContext * C) { h.OnDrawGizmos(C); })
            m_pRenderHooks->OnDrawGizmos(m_pEditorContext.get());
        if constexpr (requires(TRenderHooks & h, LevelEditorContext * C) { h.OnDrawOverlay(C); })
            m_pRenderHooks->OnDrawOverlay(m_pEditorContext.get());
    }

    void RenderEnd() override
    {
        if constexpr (requires(TUIPolicy & u, LevelEditorContext * C) { u.EndFrame(C); })
            m_pUserInteraction->EndFrame(m_pEditorContext.get());

        if constexpr (requires(TRenderHooks & h, LevelEditorContext * C) { h.OnEndFrame(C); })
            m_pRenderHooks->OnEndFrame(m_pEditorContext.get());
    }

    LevelEditorContext* Context() noexcept { return m_pEditorContext.get(); }

    IInputContext* GetInputContext() const { return m_inputHandler.get(); }

private:
    std::unique_ptr<EditorInputHandler> m_inputHandler{};
    std::unique_ptr<LevelManager>       m_pLevelManager{};
    std::unique_ptr<CommandStack>       m_pCommandStack{};
    std::unique_ptr<TStoragePolicy>     m_pStorage{};
    std::unique_ptr<TUIPolicy>          m_pUserInteraction{};
    std::unique_ptr<TUndoPolicy>        m_pUndo{};
    std::unique_ptr<TRenderHooks>       m_pRenderHooks{};

    std::unique_ptr<LevelEditorContext> m_pEditorContext{};

    std::vector<std::unique_ptr<ITool>> m_ppActiveTools{};
    std::vector<std::uint64_t>          m_selection{};
    std::uint64_t                       m_lastCreatedId{};
};

using EditorUI_ImGui = EditorUI<
    EditorStorage,
    ImGuiPolicy_Default,
    BoundedUndoPolicy,
    NoopRenderHookPolicy
>;
