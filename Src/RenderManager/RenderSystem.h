#pragma once

#include "SystemManager/ISystem.h"
#include "WindowsManager/WindowsSystem.h"

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include "IRender.h"


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

	void AttachSystemToRender(IRender* sysToRender);
	void RemoveSystemToRender(IRender* sysToRender);
	void RemoveSystemToRender(ID id);
private:
	bool QueryAndStoreAdapter();
	bool QueryAndStoreMonitorDisplay();

	bool InitDeviceAndContext();
	bool InitSwapChain();
	bool InitRenderTargetView();
	bool InitDepthAndStencilView();
	bool InitViewport();
	bool InitRasterizationState();

	void CleanBuffers();
	void SetOMStates();

	void BeginRender();
	void ExecuteRender();
	void EndRender();

private:
	WindowsSystem* m_WindowsSystem{ nullptr };

	std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> m_Adapters;
	int m_SelectedAdapterIndex{ -1 };
	UINT m_RefreshRateNumerator{ 60 };
	UINT m_RefreshRateDenominator{ 1 };
	DXGI_ADAPTER_DESC m_CurrentAdapterDesc{};

	std::vector<UINT> m_SupportedMSAA;
	UINT m_CurrentMSAA{ 8 };
	UINT m_MSAACount{ 1 };
	UINT m_MSAAQuality{ 0 };

	Microsoft::WRL::ComPtr<ID3D11Device> m_Device{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext{ nullptr };
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain{ nullptr };

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_RenderBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RasterizationState;
};
