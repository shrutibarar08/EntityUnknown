#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <vector>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "SpotLight.h"

class SpotLightManager
{
public:
	SpotLightManager(int maxSize = 5, UINT slot = 2);

	void AddLight(SpotLight* light);
	void RemoveLight(ID id);
	void Clear();

	void Build(ID3D11Device* device);
	void Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownPosition);
	void Bind(ID3D11DeviceContext* context) const;
	void UnBind(ID3D11DeviceContext* context) const;

	int GetLightCount() const;
	static float CalculateDistance(const DirectX::XMVECTOR& a, const DirectX::XMFLOAT3& b);

private:
	void BuildShadowResources(ID3D11Device* device);

private:
	std::unordered_map<ID, SpotLight*> m_Lights;
	std::vector<LightDistance> m_LightQueue;
	std::vector<SPOT_LIGHT_GPU_DATA> m_GPUData;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;

	//~ Shadow Data
	inline static bool m_bShadowTextureInitialized{ false };
	inline static Microsoft::WRL::ComPtr<ID3D11Texture2D> m_ShadowMapArray;
	inline static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShadowMapSRV;
	inline static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_ShadowMapDSVs;
	inline static std::unordered_map<int, bool> m_AssignedIndex{};
	inline static std::unordered_map<ID, int> m_LightToIndex{};

	UINT m_Slot{ 1u };
	int m_MaxBufferSize{ 10 };
	UINT m_ShadowMap_Slot{ 17u };
	UINT m_ShadowMapSize{ 2048u };
	bool m_Dirty{ false };
};
