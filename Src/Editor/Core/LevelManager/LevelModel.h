#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
#include <vector>

#include <nlohmann/json.hpp>

#include "RenderManager/Light/DefineLights.h"


enum class LevelState
{
    NOT_HOOKED,
    HOOKED
};

class Level 
{
private:
    struct LightData
    {
        std::unique_ptr<ILightSource> Light;
        bool TurnOn{ true };
    };

public:
     Level() = default;
    ~Level() = default;

    Level(const Level&) = default;
    Level(Level&&)      = default;
     
    Level& operator=(const Level&) = default;
    Level& operator=(Level&&)      = default;

    const std::string& GetName() const { return m_szName; }
    void SetName(const std::string& name) { m_szName = name; }

    void Hook();
    void UnHook();
    bool IsHooked() const { return m_eLevelState == LevelState::HOOKED; }

    //~ Light Data
    void AddLight(std::unique_ptr<ILightSource> light);
    bool RemoveLight(ILightSource* lightSource);

    bool          RemoveLight  (ID lightId);
    void          TurnOffLight (ID lightId);
    void          TurnONLight  (ID lightId);
    bool          IsLightOn    (ID lightId);
    bool          IsAValidLight(ID lightId);
    ILightSource* GetLights (ID lightId);

    std::unique_ptr<ILightSource> RemoveAndGetLight(ID lightId);
    const std::unordered_map<ID, ILightSource*>& GetLightMap();

    void LoadLevelSaveData(const nlohmann::json& levelData);
    nlohmann::json GetLevelSaveData() const;

private:
    //~ Light Helpers
    void LoadLights();
    void LoadLight(ID lightId);
    void UnLoadLights();
    void UnLoadLight(ID lightId);
    void RebuildSafeLights();

    //~ Save Helpers
    void LoadLightSaveData(const nlohmann::json& levelData);
    nlohmann::json GetLightSaveData() const;

private:
    std::string m_szName;
    std::unordered_map<ID, LightData> m_mapLights{};
    std::unordered_map<ID, ILightSource*> m_safeMapLights{};
    bool m_bDirtyLight{ true };

    LevelState m_eLevelState{ LevelState::NOT_HOOKED };
};
