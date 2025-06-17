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

	int GetLightCount() const;
	static float CalculateDistance(const DirectX::XMVECTOR& a, const DirectX::XMFLOAT3& b);
private:
	std::unordered_map<ID, SpotLight*> m_Lights;
	std::vector<LightDistance> m_LightQueue;
	std::vector<SPOT_LIGHT_GPU_DATA> m_GPUData;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;

	UINT m_Slot{ 1 };
	int m_MaxBufferSize{ 10 };
	bool m_Dirty{ false };
};
