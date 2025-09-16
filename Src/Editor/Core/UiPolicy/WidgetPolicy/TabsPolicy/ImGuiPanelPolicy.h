#pragma once
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <imgui/imgui.h>

class LevelEditorContext;

enum class PanelSlot : uint8_t
{
    Center,
    Left,
    Right,
    Bottom,
    Top
};

struct ImGuiPanelDesc
{
    std::string title;
    PanelSlot   slot = PanelSlot::Center;
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
};

struct ImGuiPanelPolicyConfig
{
    bool enableDocking = true;
    bool passthruCentralNode = true;
    bool buildDefaultLayoutOnFirstUse = false;

    float leftWidthRatio = 0.22f;
    float rightWidthRatio = 1.0f - 0.22f - 0.30f;
    float bottomHeightRatio = 0.30f;
    float topHeightRatio = 0.60f;

    ImGuiWindowFlags defaultWindowFlags =
        ImGuiWindowFlags_NoCollapse;

    std::vector<ImGuiPanelDesc> presetPanels =
    {
        { "Inspector", PanelSlot::Left },
        { "Details",   PanelSlot::Right },
        { "Create",    PanelSlot::Top },
        { "Assets",    PanelSlot::Bottom },
        { "Viewport",  PanelSlot::Center, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus }
    };
};

class ImGuiPanelPolicy
{
public:
    ImGuiPanelPolicy() = default;

    bool Init(LevelEditorContext* context) { return true; }

    bool BeginWorkspace(LevelEditorContext*);
    void EndWorkspace(LevelEditorContext*);

    bool BeginPanel(LevelEditorContext*, const char* title, PanelSlot slot,
        ImGuiWindowFlags flags = 0);
    void EndPanel(LevelEditorContext*);

    void RegisterPanel(const ImGuiPanelDesc& desc);

    void ResetLayout(LevelEditorContext*,
        std::function<void(ImGuiID, ImGuiID, ImGuiID, ImGuiID, ImGuiID, ImGuiID)> layout = nullptr);

    ImGuiPanelPolicyConfig& Config() { return m_cfg; }
    const ImGuiPanelPolicyConfig& Config() const { return m_cfg; }

    ImGuiID DockspaceId() const { return m_dockspaceId; }

    ImGuiID SlotNode(PanelSlot slot) const;

    // Slot node IDs
    ImGuiID m_nodeCenter = 0;
    ImGuiID m_nodeLeft = 0;
    ImGuiID m_nodeRight = 0;
    ImGuiID m_nodeBottom = 0;
    ImGuiID m_nodeTop = 0;

private:
    ImGuiPanelPolicyConfig m_cfg{};

    // State
    int     m_lastDockFrame = -1;
    ImGuiID m_dockspaceId = 0;
    bool    m_builtLayout = false;

    std::vector<ImGuiPanelDesc> m_registered;
};
