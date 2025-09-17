#include "EngineInspectorPolicy.h"
#include "imgui/imgui_internal.h"

#include "RenderManager/Light/DefineLights.h"
#include "SystemManager/PrimaryID.h"
#include "Editor/Core/EditorContext.h"
#include "RenderManager/DefineRenders.h"

namespace 
{
    // Lights
    inline const auto& Level_GetLightMap(Level* L) { return L->GetLightMap(); }
    inline ILightSource* Level_GetLight(Level* L, uint64_t id) { return L->GetLight(id); }
    inline bool Level_IsLightOn(Level* L, uint64_t id) { return L->IsLightOn(id); }
    inline void Level_TurnOnLight(Level* L, uint64_t id) { L->TurnONLight(id); }
    inline void Level_TurnOffLight(Level* L, uint64_t id) { L->TurnOffLight(id); }
    inline void Level_RemoveLight(Level* L, uint64_t id) { L->RemoveLight(id); }

    // Meshes
    inline const auto& Level_GetMeshMap(Level* L) { return L->GetMeshMap(); }
    inline IRender* Level_GetMesh(Level* L, uint64_t id) { return L->GetMesh(id); }
    inline void Level_RemoveMesh(Level* L, uint64_t id) { L->RemoveMesh(id); }

    // Sprites
    inline const auto& Level_GetFrontSpriteMap(Level* L) { return L->GetFrontSpriteMap(); }
    inline const auto& Level_GetBackSpriteMap(Level* L) { return L->GetBackgroundSpriteMap(); }
    inline IRender* Level_GetFrontSprite(Level* L, uint64_t id) { return L->GetFrontSprite(id); }
    inline IRender* Level_GetBackSprite(Level* L, uint64_t id) { return L->GetBackgroundSprite(id); }

    // Removal
    inline void Level_RemoveFrontSprite(Level* L, uint64_t id) { L->RemoveFrontSprite(id); }
    inline void Level_RemoveBackSprite(Level* L, uint64_t id) { L->RemoveBackgroundSprite(id); }

    // labeling
    inline const char* Render_GetName(IRender* r) { return r ? r->GetName().c_str() : ""; }
    inline const char* Render_GetType(IRender* r) { return r ? r->GetTypeName().c_str() : ""; }
}

static inline std::string ToLower(std::string s)
{
    for (auto& ch : s) ch = (char)tolower((unsigned char)ch);
    return s;
}

static inline std::string to_lower_copy(std::string s) {
    for (auto& ch : s) ch = (char)std::tolower((unsigned char)ch);
    return s;
}

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
    DrawMeshes(lvl, ctx);
    DrawSprites(lvl, ctx);
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
    ImGui::TextUnformatted("Tip: Load a level to inspect lights, meshes, and sprites.");
}

void EngineInspectorPolicy::DrawHeader(Level* lvl)
{
    (void)lvl;
    if (m_cfg.showSearch)
    {
        ImGui::SetNextItemWidth(-160.0f);
        ImGui::InputTextWithHint("##insp_search", "Search… (name or type)", m_search, IM_ARRAYSIZE(m_search));
        ImGui::SameLine();
    }

    if (m_cfg.showUtilities)
    {
        if (ImGui::Button("All Lights On"))
        {
            std::vector<uint64_t> ids;
            CollectLightIds(lvl, ids);
            for (auto id : ids) Level_TurnOnLight(lvl, id);
        }
        ImGui::SameLine();
        if (ImGui::Button("All Lights Off"))
        {
            std::vector<uint64_t> ids;
            CollectLightIds(lvl, ids);
            for (auto id : ids) Level_TurnOffLight(lvl, id);
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

    // Group by type (Directional / Spot / Point / Other)
    std::vector<uint64_t> dirIds, spotIds, pointIds, otherIds;
    dirIds.reserve(allIds.size()); spotIds.reserve(allIds.size());
    pointIds.reserve(allIds.size()); otherIds.reserve(8);

    for (auto id : allIds)
    {
        if (auto* L = lvl->GetLight(id))
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

void EngineInspectorPolicy::FilterIdsBySearch(Level* lvl, std::vector<uint64_t>& ids)
{
    if (m_search[0] == '\0') return;

    const std::string q = to_lower_copy(m_search);
    auto match = [&](ILightSource* L)->bool
        {
            if (!L) return false;
            const std::string n = to_lower_copy(L->GetLightName());
            const std::string ty = to_lower_copy(L->GetLightTypeToString());
            return n.find(q) != std::string::npos || ty.find(q) != std::string::npos;
        };

    std::vector<uint64_t> keep;
    keep.reserve(ids.size());
    for (auto id : ids)
        if (match(lvl->GetLight(id))) keep.push_back(id);

    ids.swap(keep);
}

void EngineInspectorPolicy::SortIdsByName(Level* lvl, std::vector<uint64_t>& ids)
{
    std::sort(ids.begin(), ids.end(), [&](uint64_t a, uint64_t b) {
        const auto* A = lvl->GetLight(a);
        const auto* B = lvl->GetLight(b);
        const char* an = A ? A->GetLightName().c_str() : "";
        const char* bn = B ? B->GetLightName().c_str() : "";
        return std::lexicographical_compare(an, an + std::strlen(an), bn, bn + std::strlen(bn));
        });
}

void EngineInspectorPolicy::DrawGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<uint64_t>& ids)
{
    if (ids.empty()) return;
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (auto id : ids)
            DrawLightRow(lvl, ctx, id);
    }
}

void EngineInspectorPolicy::DrawLightRow(Level* lvl, LevelEditorContext* ctx, uint64_t id)
{
    ILightSource* light = lvl->GetLight(id);
    if (!light) return;

    ImGui::PushID((int)id);

    const std::string header = light->GetLightName() + "  (" + light->GetLightTypeToString() + ")";
    const bool open = ImGui::TreeNodeEx("##hdr",
        ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen,
        "%s", header.c_str());

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

void EngineInspectorPolicy::DrawLightHeaderAndActions(Level* lvl, ILightSource* /*light*/, uint64_t id)
{
    ImGui::BeginGroup();

    const bool isOn = lvl->IsLightOn(id);
    if (ImGui::SmallButton(isOn ? "Turn Off" : "Turn On"))
    {
        if (isOn) lvl->TurnOffLight(id);
        else      lvl->TurnONLight(id);
    }
    ImGui::SameLine();

    // Remove
    if (ImGui::SmallButton("Remove"))
    {
        lvl->RemoveLight(id);
    }

    ImGui::EndGroup();
}

void EngineInspectorPolicy::DrawMeshes(Level* lvl, LevelEditorContext* ctx)
{
    std::vector<uint64_t> ids;
    CollectMeshIds(lvl, ids);
    FilterMeshIdsBySearch(lvl, ids);
    if (m_cfg.sortByName) SortMeshIdsByName(lvl, ids);

    if (!ids.empty())
        DrawMeshGroup(lvl, ctx, "Meshes", ids);
}

void EngineInspectorPolicy::CollectMeshIds(Level* lvl, std::vector<uint64_t>& out)
{
    out.clear();
    const auto& mp = Level_GetMeshMap(lvl);
    out.reserve(mp.size());
    for (const auto& [id, ptr] : mp) { (void)ptr; out.push_back(id); }
}

void EngineInspectorPolicy::FilterMeshIdsBySearch(Level* lvl, std::vector<uint64_t>& ids)
{
    if (m_search[0] == '\0') return;
    const std::string q = ToLower(m_search);
    auto match = [&](IRender* R)->bool {
        if (!R) return false;
        const std::string n = ToLower(Render_GetName(R));
        const std::string t = ToLower(Render_GetType(R));
        return n.find(q) != std::string::npos || t.find(q) != std::string::npos;
        };
    std::vector<uint64_t> keep;
    keep.reserve(ids.size());
    for (auto id : ids)
        if (match(Level_GetMesh(lvl, id))) keep.push_back(id);
    ids.swap(keep);
}

void EngineInspectorPolicy::SortMeshIdsByName(Level* lvl, std::vector<uint64_t>& ids)
{
    std::sort(ids.begin(), ids.end(), [&](uint64_t a, uint64_t b) {
        const auto* A = Level_GetMesh(lvl, a);
        const auto* B = Level_GetMesh(lvl, b);
        const char* an = A ? Render_GetName(const_cast<IRender*>(A)) : "";
        const char* bn = B ? Render_GetName(const_cast<IRender*>(B)) : "";
        return std::lexicographical_compare(an, an + std::strlen(an), bn, bn + std::strlen(bn));
        });
}

void EngineInspectorPolicy::DrawMeshGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<uint64_t>& ids)
{
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
        for (auto id : ids) DrawMeshRow(lvl, ctx, id);
}

void EngineInspectorPolicy::DrawMeshRow(Level* lvl, LevelEditorContext* ctx, uint64_t id)
{
    IRender* mesh = Level_GetMesh(lvl, id);
    if (!mesh) return;

    ImGui::PushID((int)id);

    std::string header = std::string(Render_GetName(mesh)) + "  (" + Render_GetType(mesh) + ")";
    const bool open = ImGui::TreeNodeEx("##mesh_hdr", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen, "%s", header.c_str());

    ImGui::SameLine();
    DrawMeshHeaderAndActions(lvl, mesh, id);

    if (open)
    {
        mesh->RenderControlUI(ctx);
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::PopID();
}

void EngineInspectorPolicy::DrawMeshHeaderAndActions(Level* lvl, IRender* /*mesh*/, uint64_t id)
{
    ImGui::BeginGroup();
    if (ImGui::SmallButton("Remove"))
        Level_RemoveMesh(lvl, id);
    ImGui::EndGroup();
}

void EngineInspectorPolicy::DrawSprites(Level* lvl, LevelEditorContext* ctx)
{
    std::vector<uint64_t> frontIds, backIds;

    CollectFrontSpriteIds(lvl, frontIds);
    CollectBackSpriteIds(lvl, backIds);

    FilterSpriteIdsBySearch(lvl, frontIds, /*isFront=*/true);
    FilterSpriteIdsBySearch(lvl, backIds,  /*isFront=*/false);

    if (m_cfg.sortByName) {
        SortSpriteIdsByName(lvl, frontIds, true);
        SortSpriteIdsByName(lvl, backIds, false);
    }

    if (!frontIds.empty())
        DrawSpriteGroup(lvl, ctx, "Front Sprites", frontIds, /*isFront=*/true);
    if (!backIds.empty())
        DrawSpriteGroup(lvl, ctx, "Background Sprites", backIds, /*isFront=*/false);
}

void EngineInspectorPolicy::CollectFrontSpriteIds(Level* lvl, std::vector<uint64_t>& out)
{
    out.clear();
    const auto& mp = Level_GetFrontSpriteMap(lvl);
    out.reserve(mp.size());
    for (const auto& [id, ptr] : mp) { (void)ptr; out.push_back(id); }
}

void EngineInspectorPolicy::CollectBackSpriteIds(Level* lvl, std::vector<uint64_t>& out)
{
    out.clear();
    const auto& mp = Level_GetBackSpriteMap(lvl);
    out.reserve(mp.size());
    for (const auto& [id, ptr] : mp) { (void)ptr; out.push_back(id); }
}

void EngineInspectorPolicy::FilterSpriteIdsBySearch(Level* lvl, std::vector<uint64_t>& ids, bool isFront)
{
    if (m_search[0] == '\0') return;
    const std::string q = ToLower(m_search);

    auto get = [&](uint64_t id)->IRender* {
        return isFront ? Level_GetFrontSprite(lvl, id) : Level_GetBackSprite(lvl, id);
        };

    auto match = [&](IRender* R)->bool {
        if (!R) return false;
        const std::string n = ToLower(Render_GetName(R));
        const std::string t = ToLower(Render_GetType(R));
        return n.find(q) != std::string::npos || t.find(q) != std::string::npos;
        };

    std::vector<uint64_t> keep;
    keep.reserve(ids.size());
    for (auto id : ids)
        if (match(get(id))) keep.push_back(id);
    ids.swap(keep);
}

void EngineInspectorPolicy::SortSpriteIdsByName(Level* lvl, std::vector<uint64_t>& ids, bool isFront)
{
    auto get = [&](uint64_t id)->IRender* {
        return isFront ? Level_GetFrontSprite(lvl, id) : Level_GetBackSprite(lvl, id);
        };

    std::sort(ids.begin(), ids.end(), [&](uint64_t a, uint64_t b) {
        const auto* A = get(a);
        const auto* B = get(b);
        const char* an = A ? Render_GetName(const_cast<IRender*>(A)) : "";
        const char* bn = B ? Render_GetName(const_cast<IRender*>(B)) : "";
        return std::lexicographical_compare(an, an + std::strlen(an), bn, bn + std::strlen(bn));
        });
}

void EngineInspectorPolicy::DrawSpriteGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<uint64_t>& ids, bool isFront)
{
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
        for (auto id : ids) DrawSpriteRow(lvl, ctx, id, isFront);
}

void EngineInspectorPolicy::DrawSpriteRow(Level* lvl, LevelEditorContext* ctx, uint64_t id, bool isFront)
{
    IRender* spr = isFront ? Level_GetFrontSprite(lvl, id) : Level_GetBackSprite(lvl, id);
    if (!spr) return;

    ImGui::PushID((int)id);

    std::string header = std::string(Render_GetName(spr)) + "  (" + Render_GetType(spr) + ")";
    const bool open = ImGui::TreeNodeEx("##sprite_hdr", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen, "%s", header.c_str());

    ImGui::SameLine();
    DrawSpriteHeaderAndActions(lvl, spr, id, isFront);

    if (open)
    {
        spr->RenderControlUI(ctx);
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::PopID();
}

void EngineInspectorPolicy::DrawSpriteHeaderAndActions(Level* lvl, IRender* /*sprite*/, uint64_t id, bool isFront)
{
    ImGui::BeginGroup();
    if (ImGui::SmallButton("Remove"))
    {
        if (isFront) Level_RemoveFrontSprite(lvl, id);
        else         Level_RemoveBackSprite(lvl, id);
    }
    ImGui::EndGroup();
}
