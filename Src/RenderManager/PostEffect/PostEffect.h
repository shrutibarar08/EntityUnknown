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

struct alignas(16) POSTFX_COMMON_PS_CB
{
    float                  iTime;
    float                  iTimerDelta;
    float                  iTimeFrameRate;
    int                    iFrame;

    DirectX::XMFLOAT3      iResolution;
    float                  _padRes = 0.0f;

    DirectX::XMFLOAT4      iMouse;

    DirectX::XMMATRIX      ViewMatrix;
    DirectX::XMMATRIX      ProjectionMatrix;

    DirectX::XMFLOAT3      CameraPosition;
    float                  _padCam = 0.0f;

    DirectX::XMFLOAT4      ExtraPram_1;
    DirectX::XMFLOAT4      ExtraPram_2;
    DirectX::XMFLOAT4      ExtraPram_3;
};

static_assert(sizeof(POSTFX_COMMON_PS_CB) % 16 == 0, "POSTFX_COMMON_PS_CB must be 16B aligned");

class PostEffect final : public IPostEffect
{
public:
    PostEffect() = default;
    explicit PostEffect(std::string shaderFile,
        std::string                 psEntry,
        std::string                 displayName = "main");

    ~PostEffect() override = default;

    bool Init(ID3D11Device* dev) override;
    void OnResize(uint32_t width, uint32_t height) override;
    void Update(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& cam) override;
    void RenderControlUI(LevelEditorContext* context) override;

    void Render(ID3D11Device* device,
        ID3D11DeviceContext* ctx,
        EURenderTarget& srcRT) override;

    const char* GetName() const noexcept override;
    nlohmann::json GetPostEffectSaveData() const override;
    void           LoadPostEffectSaveData(const nlohmann::json& json) override;

    void SetShaderPath(const std::string& filePath);
    const std::string& GetShaderPath() const noexcept;

    void        SetPSEntry(std::string entry);
    std::string GetPSEntry() const noexcept;

    void        SetName(std::string name);
    std::string GetFriendlyName() const noexcept;

    void SetEnabled(bool e) noexcept;
    bool IsEnabled() const noexcept;

    void RequestReload() noexcept;
    bool NeedsReload() const noexcept;

    POSTFX_COMMON_PS_CB& GetCBMutable();
    const POSTFX_COMMON_PS_CB& GetCB() const noexcept;

    void SetSharedFullscreenVS(ID3D11VertexShader* vs) noexcept override;
    ID3D11VertexShader* GetSharedFullscreenVS() const noexcept override;

private:
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_PS;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_SharedFullscreenVS;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>  m_Sampler;
    std::unique_ptr<ConstantBuffer<POSTFX_COMMON_PS_CB>> m_CommonCB;

    std::string           m_ShaderFile;
    std::string           m_PSEntry     { "main" };
    std::string           m_DisplayName { "PostEffect" };

    bool                  m_Enabled    { true };
    bool                  m_NeedsReload{ false };

    uint32_t              m_Width { 0 };
    uint32_t              m_Height{ 0 };

    POSTFX_COMMON_PS_CB   m_CBData{};
};
