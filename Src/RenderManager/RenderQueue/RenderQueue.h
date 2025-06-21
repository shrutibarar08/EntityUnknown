#pragma once

#include <d3d11.h>
#include <unordered_map>

#include "PhysicsManager/PhysicsSystem.h"
#include "RenderManager/Camera/CameraController.h"

using RENDER_MAP = std::unordered_map<ID, IRender*>;

class RenderQueueSingleton
{
public:
	static void Init(
		CameraController* controller,
		ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		PhysicsSystem* physics);

	static RenderQueueSingleton* Get();
	static void Shutdown();
	static bool IsInitialized();

	bool AddRender(IRender* render);
	bool RemoveRender(const IRender* render);
	bool RemoveRender(ID renderID);

	bool AddRenderBackground(IRender* render);
	bool RemoveRenderBackground(const IRender* render);
	bool RemoveRenderBackground(ID renderID);

	bool AddRenderFront(IRender* render);
	bool RemoveRenderFront(const IRender* render);
	bool RemoveRenderFront(ID renderID);

	bool Update(UINT width, UINT height);

	bool RenderBackground();
	bool Render();
	bool RenderFront();

	bool CleanAll();
	bool CleanBackground();
	bool CleanSpace();
	bool CleanFront();

	bool AddLight(ILightSource* light);
	bool RemoveLight(const ILightSource* light);
	bool RemoveLight(ID lightID);

	//~ Sorts so that to render in correct order
	static void ApplyPaintersAlgorithm(
		const CameraController* controller,
		const RENDER_MAP& toRenderObject,
		std::vector<ID>& sortedRenders,
		bool accountTransparentOnly=true);

private:
	RenderQueueSingleton(CameraController* controller,
		ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		PhysicsSystem* physics);

	RenderQueueSingleton(const RenderQueueSingleton&) = delete;
	RenderQueueSingleton(RenderQueueSingleton&&) = delete;
	RenderQueueSingleton& operator=(const RenderQueueSingleton&) = delete;
	RenderQueueSingleton& operator=(RenderQueueSingleton&&) = delete;

	void UpdateRenders(
		const CAMERA_INFORMATION_CPU_DESC& desc,
		const RENDER_MAP& map);

private:
	inline static std::unique_ptr<RenderQueueSingleton> m_Instance{ nullptr };

	CameraController* m_CameraController{ nullptr };
	PhysicsSystem* m_PhysicsSystem{ nullptr };
	ID3D11Device* m_Device{ nullptr };
	ID3D11DeviceContext* m_DeviceContext{ nullptr };

	//~ Data
	RENDER_MAP m_Renders{};
	RENDER_MAP m_BackgroundRenders{};
	RENDER_MAP m_FrontRenders{};

	//~ Light data
	std::unordered_map<ID, ILightSource*> m_LightSources{};

	UINT m_ScreenWidth{};
	UINT m_ScreenHeight{};
};
