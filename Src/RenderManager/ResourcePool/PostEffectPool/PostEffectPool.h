#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <shared_mutex>

#include "SystemManager/PrimaryID.h"
#include "RenderManager/Interface/IPostEffect.h"
#include "RenderManager/Components/ShaderResource/PixelShader/PixelShader.h"

typedef struct EU_POST_EFFECT_SHARED_VIEW
{
    ID                 EffectID;
    std::string        EffectName;
    EU_BLOB_INIT_DESC  BlobDesc;
    IPostEffect*       pEffect;

    bool IsValid() const { return pEffect != nullptr; }

} EU_POST_EFFECT_SHARED_VIEW;

class PostEffectPool
{
public:
    static PostEffectPool& Get()
    {
        static PostEffectPool g;
        return g;
    }

    ID   Add(const EU_POST_EFFECT_INIT_DESC& desc);
    bool Remove(ID id);

    bool Rename(ID id, const std::string& newName);

    EU_POST_EFFECT_SHARED_VIEW GetEffectByID(ID id) const;
    const std::vector<EU_POST_EFFECT_SHARED_VIEW>& GetAllEffects();
    ID IsExits(const EU_POST_EFFECT_INIT_DESC& desc);
    
private:
    PostEffectPool() = default;
    
    bool IsBlobDescSame(ID id, const EU_BLOB_INIT_DESC& right) const;

private:
    struct PostEffectData
    {
        std::string                  EffectName;
        std::unique_ptr<IPostEffect> pEffect;
        EU_BLOB_INIT_DESC            BlobDesc;
    };
    std::unordered_map<ID, PostEffectData> m_mapCachedEffectsPool;
    std::vector<EU_POST_EFFECT_SHARED_VIEW>       m_ppViewEffectsPool;
    bool m_bDirty{ false };
    mutable std::shared_mutex              m_mtx;
};
