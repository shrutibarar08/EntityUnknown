#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <vector>
#include <wrl/client.h>
#include <DirectXMath.h>

#include "PointLight.h"

class PointLightManager
{
public:
    PointLightManager(int maxSize = 5, UINT slot = 3);

    void AddLight(PointLight* light);
    void RemoveLight(ID id);
    void Clear();

    void Build(ID3D11Device* device);
    void Update(ID3D11DeviceContext* context, const DirectX::XMVECTOR& ownerPosition);
    void Bind(ID3D11DeviceContext* context) const;
    void UnBind(ID3D11DeviceContext* context) const;

    int GetLightCount() const;
    static float CalculateDistance(const DirectX::XMVECTOR& a, const DirectX::XMFLOAT3& b);

private:
    void BuildShadowResources(ID3D11Device* device);

private:
    //~ Light Data
    std::unordered_map<ID, PointLight*> m_Lights;
    std::vector<LightDistance> m_LightQueue;
    std::vector<POINT_LIGHT_GPU_DATA> m_GPUData;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;

    //~ Shadow Data
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_ShadowMapArray;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShadowMapSRV;
    std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_ShadowMapDSVs;

    UINT m_Slot{ 2 };
    UINT m_ShadowMap_Slot{ 16 };
    UINT m_ShadowMapSize{ 2048u };
    int m_MaxBufferSize;
};
