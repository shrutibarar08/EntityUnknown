#pragma once

#include <string>
#include <string_view>
#include <cstdint>

#include <wrl/client.h>
#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>
#include <nlohmann/json.hpp>

#include "RenderManager/Interface/IPostEffect.h"
#include "RenderManager/Components/ConstantBuffer.h"

class EURenderTarget;
class LevelEditorContext;
struct CAMERA_INFORMATION_CPU_DESC;

class PostEffect final : public IPostEffect
{
public:
    PostEffect(const EU_POST_EFFECT_INIT_DESC& desc);
    ~PostEffect() override = default;

    bool Init(ID3D11Device* dev) override;
    void OnResize(uint32_t width, uint32_t height) override;
    void Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam) override;
    void RenderControlUI(LevelEditorContext* context) override;

    void Render(ID3D11Device* device,
        ID3D11DeviceContext* ctx,
        EURenderTarget& srcRT) override;

    const char*    GetName() const noexcept override;
    nlohmann::json GetPostEffectSaveData() const override;

    void SetName(const std::string& name);

    void SetEnabled(bool e) noexcept override;
    bool IsEnabled() const noexcept override;

    void RequestReload() noexcept;
    bool NeedsReload() const noexcept;

    POSTFX_COMMON_PS_CB& GetCBMutable() override;
    const POSTFX_COMMON_PS_CB& GetCB() const noexcept override;
    void SetParam(const POSTFX_COMMON_PS_CB& commonData) override { m_CBData = commonData; }

    void SetSharedFullscreenVS(ID3D11VertexShader* vs) noexcept override;
    ID3D11VertexShader* GetSharedFullscreenVS() const noexcept override;

private:
    Microsoft::WRL::ComPtr<ID3D11PixelShader>            m_pPostEffectPixelShader;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>           m_SharedFullscreenVS;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>           m_Sampler;
    std::unique_ptr<ConstantBuffer<POSTFX_COMMON_PS_CB>> m_CommonCB;

    bool                     m_NeedsReload{ false };
    uint32_t                 m_Width      { 0 };
    uint32_t                 m_Height     { 0 };
    POSTFX_COMMON_PS_CB      m_CBData     {};
    bool                     m_Enabled{ true };
    EU_POST_EFFECT_INIT_DESC m_descCreateData{};
};
