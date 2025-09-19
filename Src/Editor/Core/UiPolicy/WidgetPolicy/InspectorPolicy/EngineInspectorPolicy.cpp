#include "EngineInspectorPolicy.h"
#include "imgui/imgui_internal.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "RenderManager/Light/DefineLights.h"
#include "SystemManager/PrimaryID.h"
#include "Editor/Core/EditorContext.h"
#include "RenderManager/DefineRenders.h"

#include "Editor/Core/Commands/Commands.h"


namespace
{
    // Lights
    inline const auto& Level_GetLightMap(Level* L) { return L->GetLightMap(); }
    inline ILightSource* Level_GetLight(Level* L, uint64_t id) { return L->GetLight(id); }
    inline bool Level_IsLightOn(Level* L, uint64_t id) { return L->IsLightOn(id); }
    inline void Level_TurnOnLight(Level* L, uint64_t id) { L->TurnONLight(id); }
    inline void Level_TurnOffLight(Level* L, uint64_t id) { L->TurnOffLight(id); }

    // Meshes
    inline const auto& Level_GetMeshMap(Level* L) { return L->GetMeshMap(); }
    inline IRender* Level_GetMesh(Level* L, uint64_t id) { return L->GetMesh(id); }

    // Sprites
    inline const auto& Level_GetFrontSpriteMap(Level* L) { return L->GetFrontSpriteMap(); }
    inline const auto& Level_GetBackSpriteMap(Level* L) { return L->GetBackgroundSpriteMap(); }
    inline IRender* Level_GetFrontSprite(Level* L, uint64_t id) { return L->GetFrontSprite(id); }
    inline IRender* Level_GetBackSprite(Level* L, uint64_t id) { return L->GetBackgroundSprite(id); }

    // labeling
    inline const char* Render_GetName(IRender* r) { return r ? r->GetName().c_str() : ""; }
}

static inline std::string ToLower(std::string s)
{
    for (auto& ch : s) ch = (char)tolower((unsigned char)ch);
    return s;
}

static inline std::string to_lower_copy(std::string s)
{
    for (auto& ch : s) ch = (char)std::tolower((unsigned char)ch);
    return s;
}

static inline bool RightAlignedSmallButton(const char* label)
{
    ImGuiStyle& style = ImGui::GetStyle();
    const float btnW = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;

    float curX = ImGui::GetCursorPosX();
    float availX = ImGui::GetContentRegionAvail().x;

    if (availX <= btnW + 1.0f) {
        ImGui::NewLine();
        curX = ImGui::GetCursorPosX();
        availX = ImGui::GetContentRegionAvail().x;
    }

    const float right = curX + availX;
    ImGui::SameLine();
    ImGui::SetCursorPosX(right - btnW);
    return ImGui::SmallButton(label);
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
    DrawPostChain(lvl, ctx);
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

// ------------------------------- Lights --------------------------------

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
    const auto& safeMap = Level_GetLightMap(lvl);
    out.reserve(safeMap.size());
    for (const auto& [id, ptr] : safeMap) { (void)ptr; out.push_back(id); }
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
        if (match(Level_GetLight(lvl, id))) keep.push_back(id);

    ids.swap(keep);
}

void EngineInspectorPolicy::SortIdsByName(Level* lvl, std::vector<uint64_t>& ids)
{
    std::sort(ids.begin(), ids.end(), [&](uint64_t a, uint64_t b) {
        const auto* A = Level_GetLight(lvl, a);
        const auto* B = Level_GetLight(lvl, b);
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
    ILightSource* light = Level_GetLight(lvl, id);
    if (!light) return;

    ImGui::PushID((int)id);

    const std::string& name = light->GetLightName();
    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_AllowItemOverlap;

    const bool open = ImGui::TreeNodeEx("##hdr", flags, "%s", name.c_str());

    // Actions BEFORE controls; if removed, early-out
    if (DrawLightHeaderAndActions(lvl, ctx, light, id))
    {
        if (open) ImGui::TreePop();
        ImGui::PopID();
        return;
    }

    if (open)
    {
        light->RenderControlUI(ctx);
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::PopID();
}

bool EngineInspectorPolicy::DrawLightHeaderAndActions(Level* /*lvl*/, LevelEditorContext* ctx, ILightSource* /*light*/, uint64_t id)
{
    if (RightAlignedSmallButton("Remove"))
    {
        if (ctx)
        {
            if (auto* stack = ctx->GetCommandStack())
            {
                // Undo/redo friendly removal
                stack->Execute(std::make_unique<CmdDeleteLight>(id), ctx);
                return true;
            }
        }
    }
    return false;
}

// ------------------------------- Meshes --------------------------------

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

    auto match = [&](IRender* R)->bool
        {
            if (!R) return false;
            const std::string n = ToLower(Render_GetName(R));
            return n.find(q) != std::string::npos;
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

    const std::string name = Render_GetName(mesh); // name only
    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_AllowItemOverlap;

    const bool open = ImGui::TreeNodeEx("##mesh_hdr", flags, "%s", name.c_str());

    if (DrawMeshHeaderAndActions(lvl, ctx, mesh, id))
    {
        if (open) ImGui::TreePop();
        ImGui::PopID();
        return;
    }

    if (open)
    {
        mesh->RenderControlUI(ctx);
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::PopID();
}

bool EngineInspectorPolicy::DrawMeshHeaderAndActions(Level* /*lvl*/, LevelEditorContext* ctx, IRender* /*mesh*/, uint64_t id)
{
    if (RightAlignedSmallButton("Remove"))
    {
        if (ctx)
        {
            if (auto* stack = ctx->GetCommandStack())
            {
                stack->Execute(std::make_unique<CmdRemoveMesh>(id), ctx);
                return true;
            }
        }
    }
    return false;
}

// ------------------------------- Sprites --------------------------------

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
        return n.find(q) != std::string::npos;
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

    const std::string name = Render_GetName(spr); // name only
    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_AllowItemOverlap;

    const bool open = ImGui::TreeNodeEx("##sprite_hdr", flags, "%s", name.c_str());

    if (DrawSpriteHeaderAndActions(lvl, ctx, spr, id, isFront))
    {
        if (open) ImGui::TreePop();
        ImGui::PopID();
        return;
    }

    if (open)
    {
        spr->RenderControlUI(ctx);
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::PopID();
}

bool EngineInspectorPolicy::DrawSpriteHeaderAndActions(Level* /*lvl*/, LevelEditorContext* ctx, IRender* /*sprite*/, uint64_t id, bool isFront)
{
    if (RightAlignedSmallButton("Remove"))
    {
        if (ctx)
        {
            if (auto* stack = ctx->GetCommandStack())
            {
                if (isFront) stack->Execute(std::make_unique<CmdRemoveFrontSprite>(id), ctx);
                else stack->Execute(std::make_unique<CmdRemoveBackgroundSprite>(id), ctx);
                return true;
            }
        }
    }
    return false;
}

void EngineInspectorPolicy::DrawPostChain(Level* lvl, LevelEditorContext* ctx)
{
    std::vector<ID> ids;
    CollectPostEffectIds(lvl, ids);
    FilterPostEffectIdsBySearch(lvl, ids);
    SortPostEffectIdsByName(lvl, ids);

    if (!ids.empty())
        DrawPostGroup(lvl, ctx, "Post Effects", ids);
}

void EngineInspectorPolicy::CollectPostEffectIds(Level* lvl, std::vector<ID>& out)
{
    out.clear();
    if (!lvl || !lvl->GetPostChain()) return;
    const auto& mp = lvl->GetPostChain()->GetPostChainMap();
    out.reserve(mp.size());
    for (const auto& kv : mp) out.push_back(kv.first);
}

void EngineInspectorPolicy::FilterPostEffectIdsBySearch(Level* lvl, std::vector<ID>& ids)
{
    if (m_search[0] == '\0' || !lvl || !lvl->GetPostChain()) return;
    const auto& mp = lvl->GetPostChain()->GetPostChainMap();
    const std::string q = to_lower_copy(m_search);

    auto match = [&](const EU_POST_CHAIN_SHARE_VIEW& node)->bool
        {
            const EU_POST_EFFECT_SHARED_VIEW& v = node.View;
            const std::string n = to_lower_copy(v.EffectName);
            const std::string ep = to_lower_copy(v.BlobDesc.EntryPoint);
            const std::string tg = to_lower_copy(v.BlobDesc.Target);
            const std::string p = to_lower_copy(WideToUtf8(v.BlobDesc.FilePath));
            return n.find(q) != std::string::npos ||
                ep.find(q) != std::string::npos ||
                tg.find(q) != std::string::npos ||
                p.find(q) != std::string::npos;
        };

    std::vector<ID> keep; keep.reserve(ids.size());
    for (ID id : ids)
        if (auto it = mp.find(id); it != mp.end() && match(it->second))
            keep.push_back(id);
    ids.swap(keep);
}

void EngineInspectorPolicy::SortPostEffectIdsByName(Level* lvl, std::vector<ID>& ids)
{
    if (!lvl || !lvl->GetPostChain()) return;
    const auto& mp = lvl->GetPostChain()->GetPostChainMap();
    std::sort(ids.begin(), ids.end(), [&](ID a, ID b) {
        auto ia = mp.find(a), ib = mp.find(b);
        const char* an = (ia != mp.end()) ? ia->second.View.EffectName.c_str() : "";
        const char* bn = (ib != mp.end()) ? ib->second.View.EffectName.c_str() : "";
        return std::lexicographical_compare(an, an + std::strlen(an), bn, bn + std::strlen(bn));
        });
}

void EngineInspectorPolicy::DrawPostGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<ID>& ids)
{
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
        for (ID id : ids) DrawPostEffectRow(lvl, ctx, id);
}

struct PostFxEditCache { POSTFX_COMMON_PS_CB cb{}; bool init = false; };
static std::unordered_map<ID, PostFxEditCache> g_postFxCache;

void EngineInspectorPolicy::DrawPostEffectRow(Level* lvl, LevelEditorContext* ctx, ID id)
{
    if (!lvl || !lvl->GetPostChain()) return;
    const auto& mp = lvl->GetPostChain()->GetPostChainMap();
    auto it = mp.find(id);
    if (it == mp.end()) return;

    const EU_POST_CHAIN_SHARE_VIEW& node = it->second;
    const EU_POST_EFFECT_SHARED_VIEW& v = node.View;

    ImGui::PushID((int)id);

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_AllowItemOverlap;

    // Header with name + right-aligned On checkbox (no Remove here)
    const bool open = ImGui::TreeNodeEx("##post_hdr", flags, "%s", v.EffectName.c_str());
    if (DrawPostHeaderAndActions(lvl, ctx, id, node))
    {
        if (open) ImGui::TreePop();
        ImGui::Separator();
        ImGui::PopID();
        return;
    }

    if (open)
    {
        // Read-only metadata
        ImGui::TextDisabled("Entry");  ImGui::SameLine(120.0f); ImGui::TextUnformatted(v.BlobDesc.EntryPoint.c_str());
        ImGui::TextDisabled("Target"); ImGui::SameLine(120.0f); ImGui::TextUnformatted(v.BlobDesc.Target.c_str());
        ImGui::TextDisabled("Path");   ImGui::SameLine(120.0f);
        {
            const std::string pathU8 = WideToUtf8(v.BlobDesc.FilePath);
            float wrapTo = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
            ImGui::PushTextWrapPos(wrapTo);
            ImGui::TextUnformatted(pathU8.c_str());
            ImGui::PopTextWrapPos();
        }

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::SeparatorText("Parameters");

        PostFxEditCache& cache = g_postFxCache[id];
        if (!cache.init && v.pEffect)
        {
            cache.cb = v.pEffect->GetCB();
            cache.init = true;
        }

        ImGui::PushItemWidth(260.0f);

        bool edited1 = false, edited2 = false, edited3 = false;

        ImGui::TextDisabled("ExtraPram_1"); ImGui::SameLine(120.0f);
        ImGui::DragFloat4("##p1", &cache.cb.ExtraPram_1.x, 0.01f);
        edited1 |= ImGui::IsItemEdited();

        ImGui::TextDisabled("ExtraPram_2"); ImGui::SameLine(120.0f);
        ImGui::DragFloat4("##p2", &cache.cb.ExtraPram_2.x, 0.01f);
        edited2 |= ImGui::IsItemEdited();

        ImGui::TextDisabled("ExtraPram_3"); ImGui::SameLine(120.0f);
        ImGui::DragFloat4("##p3", &cache.cb.ExtraPram_3.x, 0.01f);
        edited3 |= ImGui::IsItemEdited();

        ImGui::PopItemWidth();

        if (v.pEffect && (edited1 || edited2 || edited3))
        {
            POSTFX_COMMON_PS_CB apply = v.pEffect->GetCB();
            apply.ExtraPram_1 = cache.cb.ExtraPram_1;
            apply.ExtraPram_2 = cache.cb.ExtraPram_2;
            apply.ExtraPram_3 = cache.cb.ExtraPram_3;
            v.pEffect->SetParam(apply);
            lvl->GetPostChain()->SetNeedBuild(true);
        }

        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::PopID();
}


bool EngineInspectorPolicy::DrawPostHeaderAndActions(Level* lvl, LevelEditorContext* /*ctx*/, ID id, const EU_POST_CHAIN_SHARE_VIEW& node)
{
    ImGuiStyle& style = ImGui::GetStyle();
    const char* label = "On";
    const float labelW = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f + ImGui::GetFrameHeight();
    float curX = ImGui::GetCursorPosX();
    float availX = ImGui::GetContentRegionAvail().x;
    const float right = curX + availX;
    ImGui::SameLine();
    ImGui::SetCursorPosX(right - labelW);

    bool enabled = node.Enabled;
    if (ImGui::Checkbox(label, &enabled))
    {
        if (lvl && lvl->GetPostChain())
        {
            lvl->GetPostChain()->SetEnabled(id, enabled);
            if (auto it = lvl->GetPostChain()->GetPostChainMap().find(id);
                it != lvl->GetPostChain()->GetPostChainMap().end() && it->second.View.pEffect)
            {
                it->second.View.pEffect->SetEnabled(enabled);
            }
            lvl->GetPostChain()->SetNeedBuild(true);
        }
    }

    return false;
}