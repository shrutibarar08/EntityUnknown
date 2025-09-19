#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include <d3d11.h>
#include <wrl/client.h>
#include <nlohmann/json.hpp>

#include "RenderManager/Interface/IPostEffect.h"
#include "RenderManager/System/EURenderTarget.h"
#include "RenderManager/Interface/IRender.h"
#include "RenderManager/ResourcePool/PostEffectPool/PostEffectPool.h"


typedef struct EU_POST_CHAIN_SHARE_VIEW
{
    EU_POST_EFFECT_SHARED_VIEW View{};
    bool         Enabled{ true };
    bool         Built  { false };
} EU_POST_CHAIN_SHARE_VIEW;

class PostChain: public PrimaryID
{
public:
     PostChain();
    ~PostChain() = default;

    PostChain(const PostChain&)            = delete;
    PostChain& operator=(const PostChain&) = delete;

    // Shared fullscreen VS control
    bool InitSharedFullscreenVS(ID3D11Device* dev, const std::wstring& vsPath);
    void SetSharedFullscreenVS (ID3D11VertexShader* vs) noexcept;
    
    ID3D11VertexShader* GetSharedFullscreenVS  () const noexcept;
    bool                HasSharedFullscreenVS  () const noexcept;
    void                ClearSharedFullscreenVS() noexcept;

    bool EnsureTargets(ID3D11Device* dev, const EURenderTarget* srcRT);
    bool Resize       (ID3D11Device* dev, UINT width, UINT height, DXGI_FORMAT colorFmt);

    UINT        Width      () const noexcept;
    UINT        Height     () const noexcept;
    DXGI_FORMAT ColorFormat() const noexcept;
    size_t      Size       () const noexcept;
    void        Clear      ();
    bool        IsNeedBuild() const { return m_bDirty; }
    void        SetNeedBuild(bool flag) { m_bDirty = flag; }

    ID AddPostEffect   (const EU_POST_EFFECT_INIT_DESC& desc);
    ID AddPostEffect   (ID effectId);
    void RemovePostEffect(ID effectId);

    IPostEffect* GetPostEffect   (ID effectID) const;
    bool         IsAttachedPostEffect(ID effectID) const;

    bool SetEnabled(ID effectId, bool enabled);
    bool IsEnabled (ID effectId) const;

    bool InitAll(ID3D11Device* dev);
    void OnResizeAll(uint32_t width, uint32_t height);
    void Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam);

    // Execution
    void Execute(ID3D11Device* dev,
        ID3D11DeviceContext* ctx,
        EURenderTarget* srcRT,
        ID3D11DepthStencilState* depthDisabledState,
        ID3D11BlendState* optionalBlendState = nullptr);

    void ExecuteTo(ID3D11Device* dev,
        ID3D11DeviceContext* ctx,
        EURenderTarget* srcRT,
        EURenderTarget* destRT,
        ID3D11DepthStencilState* depthDisabledState,
        ID3D11BlendState* optionalBlendState = nullptr);

    nlohmann::json Serialize() const;
    bool Deserialize(const nlohmann::json& j);

    const std::unordered_map<ID, EU_POST_CHAIN_SHARE_VIEW>& GetPostChainMap() const { return m_mapPostEffects; }

private:
    std::unordered_map<ID, EU_POST_CHAIN_SHARE_VIEW> m_mapPostEffects;

    bool m_bDirty{ true };

    EURenderTarget m_ping;
    EURenderTarget m_pong;

    UINT        m_cachedW = 0;
    UINT        m_cachedH = 0;
    DXGI_FORMAT m_cachedFmt = DXGI_FORMAT_R8G8B8A8_UNORM;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_FullscreenVS;
    ID m_defaultID;
};
