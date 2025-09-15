#include "EngineToolPanelPolicy.h"

#include "Editor/Core/DefineTools.h"

// ---------------- Public ----------------
void EngineToolPanelPolicy::DrawTools(LevelEditorContext* ctx)
{
    // Split layout
    DrawSplit(ctx);
}

// ---------------- Split ----------------
void EngineToolPanelPolicy::DrawSplit(LevelEditorContext* ctx)
{
    ImGui::PushID(this);

    const float leftW = m_cfg.leftWidth;
    ImGui::BeginChild("##tools_left", ImVec2(leftW, 0), true);
    DrawCatalog(ctx);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##tools_right", ImVec2(0, 0), true);
    DrawInstances(ctx);
    ImGui::EndChild();

    ImGui::PopID();
}

// ---------------- Catalog ----------------
void EngineToolPanelPolicy::DrawCatalog(LevelEditorContext* ctx)
{
    (void)ctx;
    if (m_namesDirty) { RefreshNames(); m_namesDirty = false; }
    BuildFiltered();

    DrawCatalogHeader();
    DrawCatalogList(ctx);
}

void EngineToolPanelPolicy::RefreshNames()
{
    m_allNames = ToolRegistry::Get().Names();
    std::sort(m_allNames.begin(), m_allNames.end());
}

static std::string ToLower(std::string s)
{
    for (auto& ch : s) ch = (char)tolower((unsigned char)ch);
    return s;
}

void EngineToolPanelPolicy::BuildFiltered()
{
    m_filtered.clear();
    const bool hasQuery = (m_search[0] != '\0');
    const std::string q = ToLower(m_search);

    for (const auto& n : m_allNames)
    {
        if (!hasQuery || ToLower(n).find(q) != std::string::npos)
            m_filtered.push_back(n);
    }
}

void EngineToolPanelPolicy::DrawCatalogHeader()
{
    ImGui::SetNextItemWidth(-90.0f);
    ImGui::InputTextWithHint("##tool_search", "Search tools...", m_search, IM_ARRAYSIZE(m_search));
    ImGui::SameLine();
    if (ImGui::SmallButton("Reload"))
        m_namesDirty = true;

    ImGui::Separator();
}

void EngineToolPanelPolicy::DrawCatalogList(LevelEditorContext* ctx)
{
    if (m_filtered.empty())
    {
        ImGui::TextDisabled("No tools found.");
        return;
    }

    for (const auto& name : m_filtered)
    {
        ImGui::PushID(name.c_str());
        bool clicked = ImGui::Selectable(name.c_str(), false);
        ImGui::SameLine();
        if (ImGui::SmallButton("Create"))
            CreateInstance(ctx, name);

        if (clicked && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            CreateInstance(ctx, name);

        ImGui::PopID();
    }
}

// ---------------- Instances ----------------
void EngineToolPanelPolicy::DrawInstances(LevelEditorContext* ctx)
{
    DrawInstancesHeader(ctx);
    DrawInstanceList(ctx);
}

void EngineToolPanelPolicy::DrawInstancesHeader(LevelEditorContext* /*ctx*/)
{
    ImGui::TextDisabled("Instances");
    ImGui::SameLine();

    // Summary
    ImGui::Text("(%d running / %d total)",
        (int)std::count_if(m_instances.begin(), m_instances.end(), [](const Instance& i) { return i.running; }),
        (int)m_instances.size());

    ImGui::Separator();
}

void EngineToolPanelPolicy::DrawInstanceList(LevelEditorContext* ctx)
{
    // Table layout
    if (ImGui::BeginTable("##tool_instances", 4,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Tool", ImGuiTableColumnFlags_WidthStretch, 0.6f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 160.0f);
        ImGui::TableHeadersRow();

        for (auto& inst : m_instances)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            bool selected = (m_selectedId == inst.id);
            if (ImGui::Selectable(std::to_string(inst.id).c_str(), selected, ImGuiSelectableFlags_SpanAllColumns))
                m_selectedId = inst.id;

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(inst.name.c_str());

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(inst.running ? "Running" : "Stopped");

            ImGui::TableNextColumn();
            DrawInstanceRow(ctx, inst);

            // Tick after row UI to avoid re-entrancy issues with ImGui
            TickIfRunning(ctx, inst);
        }

        ImGui::EndTable();
    }
}

void EngineToolPanelPolicy::DrawInstanceRow(LevelEditorContext* ctx, Instance& inst)
{
    (void)ctx;
    ImGui::PushID(inst.id);

    if (inst.running)
    {
        if (ImGui::SmallButton("Stop")) StopInstance(inst);
    }
    else
    {
        if (ImGui::SmallButton("Start")) StartInstance(inst);
    }

    ImGui::SameLine();
    if (ImGui::SmallButton("Remove")) RemoveInstance(inst.id);

    ImGui::PopID();
}

void EngineToolPanelPolicy::TickIfRunning(LevelEditorContext* ctx, Instance& inst)
{
    if (inst.running && inst.tool)
        inst.tool->Tick(ctx);
}

// ---------------- Actions ----------------
void EngineToolPanelPolicy::CreateInstance(LevelEditorContext* ctx, const std::string& name)
{
    (void)ctx;
    auto tool = ToolRegistry::Get().CreateLevel(name);
    if (!tool) return;

    Instance inst;
    inst.id = m_nextId++;
    inst.name = name;
    inst.tool = std::move(tool);
    inst.running = true;

    m_instances.emplace_back(std::move(inst));
    if (m_cfg.autoSelectOnCreate) m_selectedId = m_instances.back().id;
}

void EngineToolPanelPolicy::RemoveInstance(int id)
{
    auto it = std::remove_if(m_instances.begin(), m_instances.end(),
        [&](const Instance& i) { return i.id == id; });
    m_instances.erase(it, m_instances.end());

    if (m_selectedId == id) m_selectedId = -1;
}

void EngineToolPanelPolicy::StartInstance(Instance& inst) { inst.running = true; }
void EngineToolPanelPolicy::StopInstance(Instance& inst) { inst.running = false; }
