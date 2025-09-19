#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
#include <vector>

#include <nlohmann/json.hpp>

#include "RenderManager/Light/DefineLights.h"
#include "RenderManager/DefineRenders.h"
#include "RenderManager/PostEffect/PostChain.h"

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
     Level();
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

    void LoadLevelSaveData(const nlohmann::json& levelData);
    nlohmann::json GetLevelSaveData() const;

    //~ Light Data
    void AddLight(std::unique_ptr<ILightSource> light);
    bool RemoveLight(ILightSource* lightSource);

    bool          RemoveLight  (ID lightId);
    void          TurnOffLight (ID lightId);
    void          TurnONLight  (ID lightId);
    bool          IsLightOn    (ID lightId);
    bool          IsAValidLight(ID lightId);
    ILightSource* GetLight     (ID lightId);

    std::unique_ptr<ILightSource> RemoveAndGetLight(ID lightId);
    const std::unordered_map<ID, ILightSource*>& GetLightMap();

    //~ Mesh Data
    bool     AddMesh     (std::unique_ptr<IRender> mesh);
    bool     RemoveMesh  (IRender* mesh);
    bool     RemoveMesh  (ID meshId);
    bool     IsAValidMesh(ID meshId);
    IRender* GetMesh     (ID meshId);

    std::unique_ptr<IRender> RemoveAndGetMesh(ID meshId);
    const std::unordered_map<ID, IRender*>& GetMeshMap();

    //~ Background Sprite Data
    bool     AddBackgroundSprite     (std::unique_ptr<IRender> sprite);
    bool     RemoveBackgroundSprite  (IRender* sprite);
    bool     RemoveBackgroundSprite  (ID spriteId);
    bool     IsAValidBackgroundSprite(ID spriteId);
    IRender* GetBackgroundSprite     (ID spriteId);

    std::unique_ptr<IRender> RemoveAndGetBackgroundSprite(ID spriteId);
    const std::unordered_map<ID, IRender*>& GetBackgroundSpriteMap();

    //~ Front Sprite Data
    bool     AddFrontSprite     (std::unique_ptr<IRender> sprite);
    bool     RemoveFrontSprite  (IRender* sprite);
    bool     RemoveFrontSprite  (ID spriteId);
    bool     IsAValidFrontSprite(ID spriteId);
    IRender* GetFrontSprite     (ID mespriteIdshId);

    std::unique_ptr<IRender> RemoveAndGetFrontSprite(ID spriteId);
    const std::unordered_map<ID, IRender*>& GetFrontSpriteMap();

    //~ Post Effect Data
    PostChain* GetPostChain() const { return m_pPostChain.get(); }

private:
    //~ Light Helpers
    void UploadLights();
    void UploadLight(ID lightId);
    void OffLoadLights();
    void OffLoadLight(ID lightId);
    void RebuildSafeLights();

    //~ Mesh Helpers
    void UploadMeshes();
    void UploadMesh(ID meshId);
    void OffLoadMeshes();
    void OffLoadMesh(ID meshId);
    void RebuildSafeMeshes();

    //~ Background Sprite Helpers
    void UploadBackgroundSprites();
    void UploadBackgroundSprite(ID lightId);
    void OffLoadBackgroundSprites();
    void OffLoadBackgroundSprite(ID lightId);
    void RebuildSafeBackgroundSprites();

    //~ Front Sprite Helpers
    void UploadFrontSprites();
    void UploadFrontSprite(ID meshId);
    void OffLoadFrontSprites();
    void OffLoadFrontSprite(ID meshId);
    void RebuildSafeFrontSprites();

    //~ Post chain related helpers
    void UploadPostChain();
    void OffloadPostChain();

    //~ Save Helpers
    void LoadLightSaveData(const nlohmann::json& levelData);
    nlohmann::json GetLightSaveData() const;

    void LoadMeshSaveData(const nlohmann::json& levelData);
    nlohmann::json GetMeshSaveData() const;

private:
    //~ Level Meta Infos
    std::string m_szName;
    LevelState m_eLevelState{ LevelState::NOT_HOOKED };

    //~ Light Meta Infos
    std::unordered_map<ID, LightData>     m_mapLights{};
    std::unordered_map<ID, ILightSource*> m_safeMapLights{};
    bool m_bDirtyLight{ true };

    //~ Mesh Meta Infos
    std::unordered_map<ID, std::unique_ptr<IRender>> m_mapMeshes{};
    std::unordered_map<ID, IRender*>                 m_safeMapMeshes{};
    bool m_bDirtyMesh{ true };

    //~ Background Meta Infos
    std::unordered_map<ID, std::unique_ptr<IRender>> m_mapBackgroundSprite{};
    std::unordered_map<ID, IRender*>                 m_safeBackgroundSprite{};
    bool m_bDirtyBackgroundSprite{ true };

    //~ Front Meta Infos
    std::unordered_map<ID, std::unique_ptr<IRender>> m_mapFrontSprite{};
    std::unordered_map<ID, IRender*>                 m_safeFrontSprite{};
    bool m_bDirtyFrontSprite{ true };

    //~ Postchain Meta Infos
    std::unique_ptr<PostChain> m_pPostChain{ nullptr };
};
