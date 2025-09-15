#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <unordered_set>
#include <algorithm>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Utils/HelperFunctions.h"
#include "SystemManager/Registry/RegistryTool.h"

class LevelEditorContext;

// ---------------------------------
// EngineToolPanelPolicy
// ---------------------------------
struct EngineToolPanelConfig
{
    float leftWidth = 260.0f;        // catalog width
    int   gridColumns = 1;           // future: tile layout
    bool  autoSelectOnCreate = true; // focus new instance
};

class EngineToolPanelPolicy
{
public:
    EngineToolPanelPolicy() = default;
    ~EngineToolPanelPolicy() = default;

    bool Init(LevelEditorContext* context) { return true; }

    // Contract expected by main policy
    void DrawTools(LevelEditorContext* ctx);

    EngineToolPanelConfig& Config() { return m_cfg; }
    const EngineToolPanelConfig& Config() const { return m_cfg; }

private:
    // ---------- small data types ----------
    struct Instance
    {
        int id = 0;                        // unique within panel
        std::string name;                  // tool display name
        std::unique_ptr<ITool> tool;      // owned instance
        bool running = true;               // tick each frame if true
    };

    // ---------- high-level steps ----------
    void DrawSplit(LevelEditorContext* ctx);
    void DrawCatalog(LevelEditorContext* ctx);
    void DrawInstances(LevelEditorContext* ctx);

    // ---------- catalog helpers ----------
    void RefreshNames();
    void BuildFiltered();
    void DrawCatalogHeader();
    void DrawCatalogList(LevelEditorContext* ctx);

    // ---------- instance helpers ----------
    void DrawInstancesHeader(LevelEditorContext* ctx);
    void DrawInstanceList(LevelEditorContext* ctx);
    void DrawInstanceRow(LevelEditorContext* ctx, Instance& inst);
    void TickIfRunning(LevelEditorContext* ctx, Instance& inst);

    // ---------- actions ----------
    void CreateInstance(LevelEditorContext* ctx, const std::string& name);
    void RemoveInstance(int id);
    void StartInstance(Instance& inst);
    void StopInstance(Instance& inst);

private:
    EngineToolPanelConfig m_cfg{};

    // catalog state
    std::vector<std::string> m_allNames;      // from registry
    std::vector<std::string> m_filtered;      // after search
    char m_search[96]{ 0 };

    // instances state
    std::vector<Instance> m_instances;
    int   m_nextId = 1;        // instance ID generator
    int   m_selectedId = -1;   // which instance row is selected

    // perf: avoid rebuilding names every frame
    bool  m_namesDirty = true;
};
