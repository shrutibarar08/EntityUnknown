#pragma once
#include <d3d11.h>
#include <unordered_map>

#include "PhysicsManager/PhysicsSystem.h"
#include "RenderManager/Camera/CameraController.h"
#include "RenderManager/Model/IModel.h"


class Render3DQueue
{
public:
	Render3DQueue(CameraController* controller, ID3D11Device* device, ID3D11DeviceContext* deviceContext, PhysicsSystem* physics);
	static bool AddModel(IModel* model);
	static bool RemoveModel(const IModel* model);
	static bool RemoveModel(uint64_t modelId);
	static bool UpdateVertexConstantBuffer(ID3D11DeviceContext* context);
	static void RenderAll(ID3D11DeviceContext* context);
	static void Clean();

	static bool AddLight(ILightSource* light);
	static bool RemoveLight(ILightSource* light);

private:
	static void SortModelZBuffer();

private:
	inline static PhysicsSystem* m_PhysicsSystem{ nullptr };
	inline static ID3D11Device* m_Device = nullptr;
	inline static ID3D11DeviceContext* m_DeviceContext = nullptr;
	inline static bool m_Initialized{ false };
	inline static CameraController* m_CameraController = nullptr;
	inline static std::unordered_map<uint64_t, IModel*> m_ModelsToRender = {};
	inline static std::vector<IModel*> m_SortedModels{};

	// TODO: Generalize light rendering mech
	inline static std::unordered_map<uint64_t, ILightSource*> m_LightsToRender = {};
};
