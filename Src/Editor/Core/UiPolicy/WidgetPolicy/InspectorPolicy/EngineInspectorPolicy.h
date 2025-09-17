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
    void   DrawLights(Level* lvl, LevelEditorContext* ctx);

    void   CollectLightIds(Level* lvl, std::vector<uint64_t>& out); // PrimaryID::ID is assumed uint64_t
    void   FilterIdsBySearch(Level* lvl, std::vector<uint64_t>& ids);
    void   SortIdsByName(Level* lvl, std::vector<uint64_t>& ids);
    void   DrawGroup(Level* lvl, LevelEditorContext* ctx, const char* label, const std::vector<uint64_t>& ids);

    void   DrawLightRow(Level* lvl, LevelEditorContext* ctx, uint64_t id);
    void   DrawLightHeaderAndActions(Level* lvl, ILightSource* light, uint64_t id);

private:
    EngineInspectorConfig m_cfg{};
    char m_search[96] = { 0 };
};
