#pragma once

#include "SystemManager/ISystem.h"
#include "WindowsManager/WindowsSystem.h"

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include "ISystemRender.h"
#include "RenderQueue/Render2DQueue.h"
#include "RenderQueue/Render3DQueue.h"


class RenderSystem final: public ISystem
{
public:
	RenderSystem(WindowsSystem* winSystem);
	~RenderSystem() override = default;

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

	bool OnInit(const SweetLoader& sweetLoader) override;
	bool OnTick(float deltaTime) override;
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
	bool InitAlphaBlendingState();	

	void ResizeSwapChain(UINT width, UINT height, bool fullscreen);

	void CleanBuffers() const;
	void SetOMStates() const;

	void BeginRender();
	void ExecuteRender();
	void EndRender();

	void TurnZBufferOn() const;
	void TurnZBufferOff() const;

private:
	WindowsSystem* m_WindowsSystem{ nullptr };
	std::unordered_map<ID, ISystemRender*> m_SystemsToRender{};
	CameraManager m_CameraManager{};
	int m_3DCameraId{ -1 };
	std::unique_ptr<Render3DQueue> m_Render3DQueue{ nullptr };
	std::unique_ptr<Render2DQueue> m_Render2DQueue{ nullptr };

	std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> m_Adapters;
	int m_SelectedAdapterIndex{ -1 };
	UINT m_RefreshRateNumerator{ 60 };
	UINT m_RefreshRateDenominator{ 1 };
	DXGI_ADAPTER_DESC m_CurrentAdapterDesc{};

	std::vector<UINT> m_SupportedMSAA;
	UINT m_MSAACount{ 1 };
	UINT m_CurrentMSAACount{ 8 };
	UINT m_MSAAQuality{ 0 };
	bool m_VSyncEnable{ false };

	Microsoft::WRL::ComPtr<ID3D11Device> m_Device{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext{ nullptr };
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain{ nullptr };

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_RenderBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthDisabledStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RasterizationState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_AlphaBlendingState;

	UINT m_PrevHeight{ 0 };
	UINT m_PrevWidth{ 0 };
};
