#include "EngineInspectorPolicy.h"
#include "imgui/imgui_internal.h"

#include "RenderManager/Light/DefineLights.h"
#include "SystemManager/PrimaryID.h"
#include "Editor/Core/EditorContext.h"

void EngineInspectorPolicy::DrawInspector(LevelEditorContext* ctx)
{
    Level* lvl = GetActiveLevel(ctx);
    if (!lvl)
    {
        DrawEmptyState();
        return;
    }

    DrawHeader(lvl);
    DrawLights(lvl, ctx);
}

Level* EngineInspectorPolicy::GetActiveLevel(LevelEditorContext* ctx)
{
    if (!ctx) return nullptr;
    return ctx->GetLevelManager()->GetActiveLevel();
}

void EngineInspectorPolicy::DrawEmptyState()
{
    ImGui::TextDisabled("No active level (or level not loaded).");
    ImGui::Separator();
    ImGui::TextUnformatted("Tip: Load a level to inspect its lights and components.");
}

void EngineInspectorPolicy::DrawHeader(Level* lvl)
{
    if (m_cfg.showSearch)
    {
        ImGui::SetNextItemWidth(-160.0f);
        ImGui::InputTextWithHint("##insp_search", "Search lights...", m_search, IM_ARRAYSIZE(m_search));
        ImGui::SameLine();
    }

    if (m_cfg.showUtilities)
    {
        if (ImGui::Button("All On"))
        {
            std::vector<uint64_t> ids;
            CollectLightIds(lvl, ids);
            for (auto id : ids) lvl->TurnONLight(id);
        }
        ImGui::SameLine();
        if (ImGui::Button("All Off"))
        {
            std::vector<uint64_t> ids;
            CollectLightIds(lvl, ids);
            for (auto id : ids) lvl->TurnOffLight(id);
        }
    }

    ImGui::Separator();
}

void EngineInspectorPolicy::DrawLights(Level* lvl, LevelEditorContext* ctx)
{
    std::vector<uint64_t> allIds;
    CollectLightIds(lvl, allIds);
    FilterIdsBySearch(lvl, allIds);
    if (m_cfg.sortByName) SortIdsByName(lvl, allIds);

    if (!m_cfg.groupByType)
    {
        DrawGroup(lvl, ctx, "Lights", allIds);
        return;
    }

    // Group by type (Directional / Spot / Point)
    std::vector<uint64_t> dirIds, spotIds, pointIds, otherIds;
    dirIds.reserve(allIds.size()); spotIds.reserve(allIds.size());
    pointIds.reserve(allIds.size()); otherIds.reserve(8);

    for (auto id : allIds)
    {
        if (auto* L = lvl->GetLights(id))
        {
            switch (L->GetLightType())
            {
            case LightType::Direction_Light: dirIds.push_back(id); break;
            case LightType::Spot_Light:      spotIds.push_back(id); break;
            case LightType::Point_Light:     pointIds.push_back(id); break;
            default:                         otherIds.push_back(id); break;
            }
        }
    }

    if (!dirIds.empty())   DrawGroup(lvl, ctx, "Directional Lights", dirIds);
    if (!spotIds.empty())  DrawGroup(lvl, ctx, "Spot Lights", spotIds);
    if (!pointIds.empty()) DrawGroup(lvl, ctx, "Point Lights", pointIds);
    if (!otherIds.empty()) DrawGroup(lvl, ctx, "Other Lights", otherIds);
}

void EngineInspectorPolicy::CollectLightIds(Level* lvl, std::vector<uint64_t>& out)
{
    out.clear();
    const auto& safeMap = lvl->GetLightMap();
    out.reserve(safeMap.size());
    for (const auto& [id, ptr] : safeMap)
    {
        (void)ptr;
        out.push_back(id);
    }
}

static inline std::string ToLower(std::string s)
{
    for (auto& ch : s) ch = (char)tolower((unsigned char)ch);
    return s;
}

void EngineInspectorPolicy::FilterIdsBySearch(Level* lvl, std::vector<uint64_t>& ids)
{
    if (m_search[0] == '\0') return;

    const std::string q = ToLower(m_search);
    auto match = [&](ILightSource* L)->bool
        {
            if (!L) return false;
            const std::string n = ToLower(L->GetLightName());
            const std::string ty = ToLower(L->GetLightTypeToString());
            return n.find(q) != std::string::npos || ty.find(q) != std::string::npos;
        };

    std::vector<uint64_t> keep;
    keep.reserve(ids.size());
    for (auto id : ids)
        if (match(lvl->GetLights(id))) keep.push_back(id);

    ids.swap(keep);
}

void EngineInspectorPolicy::SortIdsByName(Level* lvl, std::vector<uint64_t>& ids)
{
    std::sort(ids.begin(), ids.end(), [&](uint64_t a, uint64_t b) {
        const auto* A = lvl->GetLights(a);
        const auto* B = lvl->GetLights(b);
        const char* an = A ? A->GetLightName().c_str() : "";
        const char* bn = B ? B->GetLightName().c_str() : "";
        return std::lexicographical_compare(an, an + std::strlen(an), bn, bn + std::strlen(bn));
        });
}

void EngineInspectorPolicy::DrawGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<uint64_t>& ids)
{
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (auto id : ids)
            DrawLightRow(lvl, ctx, id);
    }
}

void EngineInspectorPolicy::DrawLightRow(Level* lvl, LevelEditorContext* ctx, uint64_t id)
{
    ILightSource* light = lvl->GetLights(id);
    if (!light) return;

    ImGui::PushID((int)id);

    const std::string header = light->GetLightName() + "  (" + light->GetLightTypeToString() + ")";
    const bool open = ImGui::TreeNodeEx("##hdr", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen, "%s", header.c_str());

    ImGui::SameLine();
    DrawLightHeaderAndActions(lvl, light, id);

    if (open)
    {
        light->RenderControlUI(ctx);
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::PopID();
}

void EngineInspectorPolicy::DrawLightHeaderAndActions(Level* lvl, ILightSource* light, uint64_t id)
{
    (void)light;

    ImGui::BeginGroup();

    // On/Off toggle
    bool isOn = lvl->IsLightOn(id);
    if (ImGui::SmallButton(isOn ? "Turn Off" : "Turn On"))
    {
        if (isOn) lvl->TurnOffLight(id);
        else      lvl->TurnONLight(id);
    }
    ImGui::SameLine();

    if (ImGui::SmallButton("Remove"))
    {
        lvl->RemoveLight(id);
    }

    ImGui::EndGroup();
}
