#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <vector>
#include <wrl/client.h>

#include "DirectionalLight.h"


class DirectionalLightManager
{
public:
	DirectionalLightManager(int maxSize, UINT slot);

	void AddLight(DirectionalLight* light);
	void RemoveLight(ID id);
	void Clear();

	void Build(ID3D11Device* device);
	void Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownerPosition);
	void Bind(ID3D11DeviceContext* context) const;
	void UnBind(ID3D11DeviceContext* context) const;

	int GetLightCount() const { return static_cast<int>(m_Lights.size()); }

private:
	void BuildShadowResources(ID3D11Device* device) const;

private:
	//~ Light Data
	std::unordered_map<ID, DirectionalLight*> m_Lights;
	std::vector<DIRECTIONAL_LIGHT_GPU_DATA> m_GPUData;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;

	//~ Shadow Data
	inline static bool m_bShadowTextureInitialized{ false };
	inline static Microsoft::WRL::ComPtr<ID3D11Texture2D> m_ShadowMapArray;
	inline static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShadowMapSRV;
	inline static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_ShadowMapDSVs;
	inline static std::unordered_map<int, bool> m_AssignedIndex{};
	inline static std::unordered_map<ID, int> m_LightToIndex{};

	UINT m_Slot{ 0 };
	UINT m_ShadowMap_Slot{ 15 };
	int m_MaxBufferSize{ 10 };
	UINT m_ShadowMapSize{ 2048u };
};
