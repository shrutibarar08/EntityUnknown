// EngineInspectorPolicy.h

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <optional>

#include "imgui/imgui.h"

class LevelEditorContext;
class Level;
class ILightSource;
class IRender;

struct EngineInspectorConfig
{
    bool showSearch = true;
    bool groupByType = true;
    bool sortByName = true;
    bool showUtilities = true;
};

class EngineInspectorPolicy
{
public:
    EngineInspectorPolicy() = default;

    bool Init(LevelEditorContext* context) { return true; }
    void DrawInspector(LevelEditorContext* ctx);

    EngineInspectorConfig& Config() { return m_cfg; }
    const EngineInspectorConfig& Config() const { return m_cfg; }

private:
    Level* GetActiveLevel(LevelEditorContext* ctx);
    void   DrawEmptyState();
    void   DrawHeader(Level* lvl);

    // -------- Lights --------
    void   DrawLights(Level* lvl, LevelEditorContext* ctx);
    void   CollectLightIds(Level* lvl, std::vector<uint64_t>& out);
    void   FilterIdsBySearch(Level* lvl, std::vector<uint64_t>& ids);
    void   SortIdsByName(Level* lvl, std::vector<uint64_t>& ids);
    void   DrawGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<uint64_t>& ids);
    void   DrawLightRow(Level* lvl, LevelEditorContext* ctx, uint64_t id);
    void   DrawLightHeaderAndActions(Level* lvl, ILightSource* light, uint64_t id);

    // Meshes
    void   DrawMeshes(Level* lvl, LevelEditorContext* ctx);
    void   CollectMeshIds(Level* lvl, std::vector<uint64_t>& out);
    void   FilterMeshIdsBySearch(Level* lvl, std::vector<uint64_t>& ids);
    void   SortMeshIdsByName(Level* lvl, std::vector<uint64_t>& ids);
    void   DrawMeshGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<uint64_t>& ids);
    void   DrawMeshRow(Level* lvl, LevelEditorContext* ctx, uint64_t id);
    void   DrawMeshHeaderAndActions(Level* lvl, IRender* mesh, uint64_t id);

    // Sprites
    void   DrawSprites(Level* lvl, LevelEditorContext* ctx);
    void   CollectFrontSpriteIds(Level* lvl, std::vector<uint64_t>& out);
    void   CollectBackSpriteIds(Level* lvl, std::vector<uint64_t>& out);
    void   FilterSpriteIdsBySearch(Level* lvl, std::vector<uint64_t>& ids, bool isFront);
    void   SortSpriteIdsByName(Level* lvl, std::vector<uint64_t>& ids, bool isFront);
    void   DrawSpriteGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<uint64_t>& ids, bool isFront);
    void   DrawSpriteRow(Level* lvl, LevelEditorContext* ctx, uint64_t id, bool isFront);
    void   DrawSpriteHeaderAndActions(Level* lvl, IRender* sprite, uint64_t id, bool isFront);

private:
    EngineInspectorConfig m_cfg{};
    char m_search[96] = { 0 };
};
