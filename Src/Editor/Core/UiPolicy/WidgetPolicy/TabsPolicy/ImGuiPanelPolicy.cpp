#include "ImGuiPanelPolicy.h"
#include <imgui/imgui_internal.h>
#include <algorithm>

static ImGuiID& slot_ref(ImGuiPanelPolicy* self, PanelSlot slot)
{
    switch (slot)
    {
    case PanelSlot::Left:   return self->m_nodeLeft;
    case PanelSlot::Right:  return self->m_nodeRight;
    case PanelSlot::Bottom: return self->m_nodeBottom;
    case PanelSlot::Top:    return self->m_nodeTop;
    case PanelSlot::Center: default: return self->m_nodeCenter;
    }
}

ImGuiID ImGuiPanelPolicy::SlotNode(PanelSlot slot) const
{
    switch (slot)
    {
    case PanelSlot::Left:   return m_nodeLeft;
    case PanelSlot::Right:  return m_nodeRight;
    case PanelSlot::Bottom: return m_nodeBottom;
    case PanelSlot::Top:    return m_nodeTop;
    case PanelSlot::Center: default: return m_nodeCenter;
    }
}

void ImGuiPanelPolicy::RegisterPanel(const ImGuiPanelDesc& desc)
{
    // if same title exists, update; else push
    auto it = std::find_if(m_registered.begin(), m_registered.end(),
        [&](const ImGuiPanelDesc& d) { return d.title == desc.title; });
    if (it != m_registered.end()) *it = desc;
    else m_registered.push_back(desc);
}

bool ImGuiPanelPolicy::BeginWorkspace(LevelEditorContext*)
{
    if (!m_cfg.enableDocking) return false;

    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == 0)
        return false;

    const int frame = ImGui::GetFrameCount();
    if (m_lastDockFrame == frame)
        return true;

    ImGuiViewport* vp = ImGui::GetMainViewport();
    m_dockspaceId = vp->ID;

    ImGuiDockNodeFlags flags = 0;
    if (m_cfg.passthruCentralNode)
        flags |= ImGuiDockNodeFlags_PassthruCentralNode;

    ImGui::DockSpaceOverViewport(m_dockspaceId, vp, flags);

    // First-time default layout?
    if (m_cfg.buildDefaultLayoutOnFirstUse && !m_builtLayout)
    {
        if (ImGui::DockBuilderGetNode(m_dockspaceId))
        {
            ImGui::DockBuilderRemoveNode(m_dockspaceId);
            ImGui::DockBuilderAddNode(m_dockspaceId, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(m_dockspaceId, vp->Size);

            ImGuiID root = m_dockspaceId;
            // Split order: root -> (Left, Right), then split Right -> Bottom
            // and Left -> Top to emulate common editor layouts.
            ImGuiID right = 0;
            m_nodeLeft = ImGui::DockBuilderSplitNode(root, ImGuiDir_Left, m_cfg.leftWidthRatio, nullptr, &right);
            m_nodeBottom = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, m_cfg.bottomHeightRatio, nullptr, &right);
            m_nodeTop = ImGui::DockBuilderSplitNode(m_nodeLeft, ImGuiDir_Up, m_cfg.topHeightRatio, nullptr, &m_nodeLeft);

            // whatever remains is center & right
            m_nodeRight = right;
            m_nodeCenter = right; // by default, center is the big right region’s central node

            // Dock any pre-registered panels
            for (const auto& p : (!m_registered.empty() ? m_registered : m_cfg.presetPanels))
            {
                const ImGuiID node = SlotNode(p.slot);
                if (node != 0 && !p.title.empty())
                    ImGui::DockBuilderDockWindow(p.title.c_str(), node);
            }

            ImGui::DockBuilderFinish(root);
            m_builtLayout = true;
        }
    }

    m_lastDockFrame = frame;
    return true;
}

void ImGuiPanelPolicy::EndWorkspace(LevelEditorContext* /*ctx*/)
{
    // nothing for now
}

bool ImGuiPanelPolicy::BeginPanel(LevelEditorContext* /*ctx*/,
    const char* title,
    PanelSlot /*slot*/,
    ImGuiWindowFlags flags)
{
    const ImGuiWindowFlags finalFlags = (flags == 0) ? m_cfg.defaultWindowFlags : flags;
    return ImGui::Begin(title, nullptr, finalFlags);
}

void ImGuiPanelPolicy::EndPanel(LevelEditorContext* /*ctx*/)
{
    ImGui::End();
}

void ImGuiPanelPolicy::ResetLayout(
    LevelEditorContext* /*ctx*/,
    std::function<void(ImGuiID, ImGuiID, ImGuiID, ImGuiID, ImGuiID, ImGuiID)> layout)
{
    if (m_dockspaceId == 0) return;

    ImGui::DockBuilderRemoveNode(m_dockspaceId);
    ImGui::DockBuilderAddNode(m_dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(m_dockspaceId, ImGui::GetMainViewport()->Size);

    ImGuiID root = m_dockspaceId;

    ImGuiID right = 0;
    m_nodeLeft = ImGui::DockBuilderSplitNode(root, ImGuiDir_Left, m_cfg.leftWidthRatio, nullptr, &right);
    m_nodeBottom = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, m_cfg.bottomHeightRatio, nullptr, &right);
    m_nodeTop = ImGui::DockBuilderSplitNode(m_nodeLeft, ImGuiDir_Up, m_cfg.topHeightRatio, nullptr, &m_nodeLeft);

    m_nodeRight = right;
    m_nodeCenter = right;

    if (layout) layout(root, m_nodeLeft, m_nodeRight, m_nodeBottom, m_nodeTop, m_nodeCenter);

    // If no custom layout provided, dock registered/preset panels
    if (!layout)
    {
        const auto& list = (!m_registered.empty() ? m_registered : m_cfg.presetPanels);
        for (const auto& p : list)
        {
            if (!p.title.empty())
                ImGui::DockBuilderDockWindow(p.title.c_str(), SlotNode(p.slot));
        }
    }

    ImGui::DockBuilderFinish(root);
}
