#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
#include <vector>

#include "RenderManager/Light/DefineLights.h"

enum class LevelState
{
    UNLOADED,
    LOADED
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

    void Load();
    void UnLoad();
    bool IsLoaded() const { return m_eLevelState == LevelState::LOADED; }

    //~ Light Data
    void AddLight(std::unique_ptr<ILightSource> light);
    bool RemoveLight(ILightSource* lightSource);

    bool          RemoveLight  (ID lightId);
    void          TurnOffLight (ID lightId);
    void          TurnONLight  (ID lightId);
    bool          IsLightOn    (ID lightId);
    bool          IsAValidLight(ID lightId);
    ILightSource* GetLightData (ID lightId);

    std::unique_ptr<ILightSource> RemoveAndGetLight(ID lightId);
    const std::unordered_map<ID, ILightSource*>& GetLightMapData();

private:
    void LoadLights();
    void LoadLight(ID lightId);
    void UnLoadLights();
    void UnLoadLight(ID lightId);
    void RebuildSafeLights();

private:
    std::string m_szName;
    std::unordered_map<ID, LightData> m_mapLights{};
    std::unordered_map<ID, ILightSource*> m_safeMapLights{};
    bool m_bDirtyLight{ true };

    LevelState m_eLevelState{ LevelState::UNLOADED };
};
