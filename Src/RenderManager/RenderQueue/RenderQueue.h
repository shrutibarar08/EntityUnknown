#pragma once

#include <d3d11.h>
#include <unordered_map>

#include "PhysicsManager/PhysicsSystem.h"
#include "RenderManager/Camera/CameraController.h"
#include "RenderManager/Frustum/Frustum.h"

#include "RenderManager/Interface/IPostEffect.h"
#include "RenderManager/Interface/IRender.h"
#include "RenderManager/Interface/ILightSource.h"

#include "RenderManager/PostEffect/PostChain.h"
#include "RenderManager/System/EURenderTarget.h"

using RENDER_MAP = std::unordered_map<ID, IRender*>;


class RenderQueue
{
public:
	static void Init(
		CameraController* controller,
		ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		PhysicsSystem* physics,
		EURenderTarget* renderTarget);

	static RenderQueue* Get();
	static void Shutdown();
	static bool IsInitialized();

	CameraController* GetCameraController() const;

	void Tick(float deltaTime);
	//~ Objects
	bool AddRender(IRender* render);
	bool RemoveRender(const IRender* render);
	bool RemoveRender(ID renderID);

	//~ Sprites
	bool AddRenderBackground(IRender* render);
	bool RemoveRenderBackground(const IRender* render);
	bool RemoveRenderBackground(ID renderID);

	bool AddRenderFront(IRender* render);
	bool RemoveRenderFront(const IRender* render);
	bool RemoveRenderFront(ID renderID);

	bool Update(UINT width, UINT height);

	//~ Render
	bool RenderBackground();
	bool Render();
	bool RenderFront();
	bool RenderShadowCast();
	bool RenderPostEffects(EURenderTarget* src, ID3D11DepthStencilState* depth);

	bool UnBind();

	bool CleanAll();
	bool CleanBackground();
	bool CleanSpace();
	bool CleanFront();

	//~ Lights
	bool AddLight(ILightSource* light);
	bool RemoveLight(const ILightSource* light);
	bool RemoveLight(ID lightID);
	bool UpdateLight();

	//~ Post Effects
	void SetPostChain	 (PostChain* postChain);
	void RemovePostChain (PostChain* postChain);
	bool UpdatePostEffect(float deltaTime, const CAMERA_INFORMATION_CPU_DESC& desc);

	ID3D11Device*		 GetDevice		 () const { return m_Device;		}
	ID3D11DeviceContext* GetDeviceContext() const { return m_DeviceContext; }

private:
	RenderQueue(CameraController* controller,
		ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		PhysicsSystem* physics,
		EURenderTarget* renderTarget);

	RenderQueue(const RenderQueue&) = delete;
	RenderQueue(RenderQueue&&) = delete;
	RenderQueue& operator=(const RenderQueue&) = delete;
	RenderQueue& operator=(RenderQueue&&) = delete;

	void UpdateRenders(
		const CAMERA_INFORMATION_CPU_DESC& desc,
		const RENDER_MAP& map);

	void ApplyPaintersAlgorithm(
		const CameraController* controller,
		const RENDER_MAP& toRenderObject,
		std::vector<ID>& sortedRenders,
		bool accountTransparentOnly = true) const;

	bool IsInside(IRender* render) const;

	void SetRenderTargetToShadowMap(ID3D11DepthStencilView* dsv) const;
	void ClearDepthStencilView(ID3D11DepthStencilView* dsv) const;

	//~ Get All Objects
	RENDER_MAP& GetRenders() { return m_Renders; }
	RENDER_MAP& GetBackgroundRenders() { return m_BackgroundRenders; }
	RENDER_MAP& GetFrontRenders() { return m_FrontRenders; }

	std::unordered_map<ID, ILightSource*>& GetLights() { return m_LightSources; }

	void BuildPostProcessing();

private:
	static constexpr UINT DEFAULT_SHADOW_MAP_SIZE = 2048u;
	inline static std::unique_ptr<RenderQueue> m_Instance{ nullptr };

	CameraController*	 m_CameraController{ nullptr };
	PhysicsSystem*		 m_PhysicsSystem{ nullptr };
	ID3D11Device*		 m_Device{ nullptr };
	ID3D11DeviceContext* m_DeviceContext{ nullptr };
	Frustum				 m_Frustum{};
	EURenderTarget*		 m_RenderTarget{ nullptr };

	//~ Data
	RENDER_MAP m_Renders{};
	RENDER_MAP m_BackgroundRenders{};
	RENDER_MAP m_FrontRenders{};

	//~ Light data
	std::unordered_map<ID, ILightSource*> m_LightSources{};

	UINT m_ScreenWidth{};
	UINT m_ScreenHeight{};

	PostChain* m_SelectedPostChain{ nullptr };
};
