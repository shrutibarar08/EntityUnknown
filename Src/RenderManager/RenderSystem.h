#pragma once

#include "ISystemRender.h"
#include "SystemManager/ISystem.h"
#include "WindowsManager/WindowsSystem.h"
#include "PhysicsManager/PhysicsSystem.h"
#include "Camera/CameraController.h"

#include "RenderManager/System/RenderAdapter.h"
#include "RenderManager/System/RenderDisplaySetting.h"
#include "RenderManager/System/RenderDevice.h"
#include "RenderManager/System/EURenderTarget.h"

#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "PostEffect/PostChain.h"


class RenderSystem final: public ISystem
{
public:
	RenderSystem(WindowsSystem* winSystem, PhysicsSystem* physics);
	~RenderSystem() override = default;

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

	bool OnInit(const SweetLoader& sweetLoader) override;
	bool OnFrameUpdate(float deltaTime) override;
	bool OnExit(SweetLoader& sweetLoader) override;
	std::string GetSystemName() override;

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;

	void AttachSystemToRender(ISystemRender* sysToRender);
	void RemoveSystemToRender(const ISystemRender* sysToRender);
	void RemoveSystemToRender(ID id);

	//~ Helper Functions
	DXGI_ADAPTER_DESC GetAdapterInformation() const;
	float GetRefreshRate() const;
	UINT GetSelectedMSAA() const;

	std::vector<UINT> GetAvailableMSAAs() const;

	bool IsVSyncEnabled() const { return m_VSyncEnable; }
	void SetVSync(bool flag) { m_VSyncEnable = flag; }

	CameraController* GetCameraController() const;

private:
	bool SetMSAA(UINT msaaValue);

	bool BuildRenderer();
	bool BuildViewsAndStates(bool buildSwapChain=false);

	bool QueryAndStoreAdapter();
	bool QueryAndStoreMonitorDisplay();
	bool QueryAndStoreMSAA();

	bool InitDeviceAndContext();
	bool InitSwapChain();
	bool InitRenderTargetView();
	bool InitDepthAndStencilView();
	bool InitViewport() const;
	bool InitRasterizationState();
	bool InitDepthRasterizationState();
	bool InitAlphaBlendingState();	

	void ResizeSwapChain(UINT width, UINT height, bool fullscreen);

	void CleanMainRTV();
	void BindMainRTV();

	void BeginRender();
	void ExecuteRender();
	void EndRender();

	void TurnZBufferOn() const;
	void TurnZBufferOff() const;
	void TurnZBufferReadOnly() const;
	void SetAlphaBlendState() const;

	bool CreateTestEffectRT();
	bool InitPostFX();

private:
	WindowsSystem* m_WindowsSystem{ nullptr };
	PhysicsSystem* m_PhysicsSystem{ nullptr };
	std::unordered_map<ID, ISystemRender*> m_SystemsToRender{};
	CameraManager m_CameraManager{};
	int m_3DCameraId{ -1 };

	RenderAdapter		 m_Adapter{};
	RenderMonitorSetting m_Monitor{};
	RenderDevice		 m_Device {};
	EURenderTarget		 m_MainRT {};

	std::vector<UINT> m_SupportedMSAA;
	UINT m_MSAACount{ 1 };
	UINT m_CurrentMSAACount{ 8 };
	UINT m_MSAAQuality{ 0 };
	bool m_VSyncEnable{ false };

	Microsoft::WRL::ComPtr<IDXGISwapChain>			m_SwapChain{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthDisabledStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthReadOnlyState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_RasterizationState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_DepthRasterizationState;
	Microsoft::WRL::ComPtr<ID3D11BlendState>		m_AlphaBlendingState;

	//~ Test Post Effects
	// Offscreen RTT for post
	EURenderTarget m_EffectRT;

	// PostFX resources
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_FullscreenVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_PS_BoxBlur;   // choose one to test
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_PS_Sepia;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_LinearClamp;

	// constants: 16 bytes is enough (float2 invTexel + padding)
	Microsoft::WRL::ComPtr<ID3D11Buffer>       m_PostCB;

	UINT m_PrevHeight{ 0 };
	UINT m_PrevWidth { 0 };

	//~ Test
	std::unique_ptr<PostChain> m_PostChain;
	bool m_ShowPostFXUI = true;
};
