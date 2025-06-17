#pragma once
#include <d3d11.h>
#include <memory>

#include "ILightSource.h"
#include "DirectionalLight/DirectionalLightManager.h"
#include "SpotLight/SpotLightManager.h"
#include "PointLight/PointLightManager.h"

typedef struct LIGHT_META_DATA
{
	int DirectionLightCount;
	int SpotLightCount;
	int PointLightCount;
}LIGHT_META_DATA;

class LightManager
{
public:
	LightManager() = default;
	~LightManager() = default;

	LightManager(const LightManager&) = delete;
	LightManager(LightManager&&) = delete;
	LightManager& operator=(const LightManager&) = delete;
	LightManager& operator=(LightManager&&) = delete;

	void SetDirectionalLightBufferSize(int bufferSize, int slot=0);
	void SetSpotLightBufferSize(int bufferSize, int slot=1);
	void SetPointLightBufferSize(int bufferSize, int slot=2);

	void SetActiveDirectionalLight(bool flag);
	void SetActiveSpotLight(bool flag);
	void SetActivePointLight(bool flag);

	void AddLight(ILightSource* light) const;
	void RemoveLight(ILightSource* light) const;
	void Clear() const;

	void Build(ID3D11Device* device);
	void Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownerPosition) const;
	void Bind(ID3D11DeviceContext* context) const;

	LIGHT_META_DATA GetLightMetaDataInfo() const;
private:
	std::unique_ptr<DirectionalLightManager> m_DirectionalLightManager{ nullptr };
	int m_DirectionalBufferSize{ 10 };
	int m_DirectionalBuffer_Slot{ 0 };
	bool m_bActiveDirectionalLight{ true };

	std::unique_ptr<SpotLightManager> m_SpotLightManager{ nullptr };
	int m_SpotLightBufferSize{ 10 };
	int m_SpotLightBuffer_Slot{ 1 };
	bool m_bActiveSpotLight{ true };

	std::unique_ptr<PointLightManager> m_PointLightManager{ nullptr };
	int m_PointLightBufferSize{ 10 };
	int m_PointLightBuffer_Slot{ 2 };
	bool m_bActivePointLight{ true };

	bool m_Initialized{ false };
};
