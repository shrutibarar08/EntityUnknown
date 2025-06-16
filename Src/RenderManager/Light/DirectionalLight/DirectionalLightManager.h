#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <vector>
#include <wrl/client.h>

#include "DirectionalLight.h"


class DirectionalLightManager
{
public:
	DirectionalLightManager(UINT slot, int maxSize = 5);

	void AddLight(DirectionalLight* light);
	void RemoveLight(ID id);
	void Clear();

	void Build(ID3D11Device* device);
	void Update(ID3D11DeviceContext* context);
	void Bind(ID3D11DeviceContext* context) const;

	int GetLightCount() const { return static_cast<int>(m_Lights.size()); }

private:
	std::unordered_map<ID, DirectionalLight*> m_Lights;
	std::vector<DIRECTIONAL_LIGHT_GPU_DATA> m_GPUData;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;

	UINT m_Slot;
	int m_MaxBufferSize;
};
