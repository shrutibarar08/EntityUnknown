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

class PostChain
{
public:
    struct Stats { size_t total = 0; size_t enabled = 0; };

    PostChain() = default;
    ~PostChain() = default;

    PostChain(const PostChain&) = delete;
    PostChain& operator=(const PostChain&) = delete;

    // Shared fullscreen VS control
    bool InitSharedFullscreenVS(ID3D11Device* dev, const std::wstring& vsPath);
    void SetSharedFullscreenVS(ID3D11VertexShader* vs) noexcept;
    ID3D11VertexShader* GetSharedFullscreenVS() const noexcept;
    bool HasSharedFullscreenVS() const noexcept;
    void ClearSharedFullscreenVS() noexcept;

    bool EnsureTargets(ID3D11Device* dev, const EURenderTarget& srcRT);
    bool Resize(ID3D11Device* dev, UINT width, UINT height, DXGI_FORMAT colorFmt);

    UINT  Width()  const noexcept;
    UINT  Height() const noexcept;
    DXGI_FORMAT ColorFormat() const noexcept;

    // CRUD
    std::string Add(std::unique_ptr<IPostEffect> fx, std::string name, bool enabled = true);
    std::string InsertAt(std::unique_ptr<IPostEffect> fx, std::string name, size_t index, bool enabled = true);
    std::unique_ptr<IPostEffect> Replace(const std::string& name, std::unique_ptr<IPostEffect> fx);
    std::unique_ptr<IPostEffect> Remove(const std::string& name);
    void Clear();

    IPostEffect* Get(const std::string& name) const;
    IPostEffect* GetAt(size_t index) const;
    bool                       Exists(const std::string& name) const;
    size_t                     Size() const noexcept;
    const std::vector<std::string>& GetOrder() const noexcept;

    bool MoveTo(const std::string& name, size_t newIndex);
    bool SetOrder(const std::vector<std::string>& newOrder);

    bool SetEnabled(const std::string& name, bool enabled);
    bool IsEnabled(const std::string& name) const;

    Stats GetStats() const;

    bool InitAll(ID3D11Device* dev);
    void OnResizeAll(uint32_t width, uint32_t height);
    void Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam);

    // Execution
    void Execute(ID3D11Device* dev,
        ID3D11DeviceContext* ctx,
        EURenderTarget& srcRT,
        ID3D11DepthStencilState* depthDisabledState,
        ID3D11BlendState* optionalBlendState = nullptr);

    void ExecuteTo(ID3D11Device* dev,
        ID3D11DeviceContext* ctx,
        EURenderTarget& srcRT,
        EURenderTarget& destRT,
        ID3D11DepthStencilState* depthDisabledState,
        ID3D11BlendState* optionalBlendState = nullptr);

    nlohmann::json Serialize() const;
    bool Deserialize(const nlohmann::json& j, ID3D11Device* dev);

private:
    std::string UniqueName(std::string base) const;

    struct Node
    {
        std::unique_ptr<IPostEffect> fx;
        bool enabled = true;
    };

    std::unordered_map<std::string, Node> m_nodes;
    std::vector<std::string>              m_order;

    EURenderTarget m_ping;
    EURenderTarget m_pong;

    UINT        m_cachedW = 0;
    UINT        m_cachedH = 0;
    DXGI_FORMAT m_cachedFmt = DXGI_FORMAT_R8G8B8A8_UNORM;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_FullscreenVS;
};
