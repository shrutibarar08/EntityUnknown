#pragma once
#include "SystemManager/PrimaryID.h"
#include <DirectXMath.h>
#include <string>
#include <d3d11.h>
#include <wrl/client.h>

struct LightDistance
{
	ID id;
	float distance;

	bool operator<(const LightDistance& rhs) const
	{
		return distance < rhs.distance;
	}
};

enum class LightType : uint8_t
{
	Direction_Light,
	Spot_Light,
	Point_Light
};

typedef struct INITIALIZE_LIGHT_SOURCE_DESC
{
	ID3D11Device* Device;
	UINT Width;
	UINT Height;
}INITIALIZE_LIGHT_SOURCE_DESC;

class ILightSource: public PrimaryID
{
public:
	ILightSource()									= default;
	virtual ~ILightSource()							= default;
	ILightSource(const ILightSource&)				= default;
	ILightSource(ILightSource&&)					= default;
	ILightSource& operator=(const ILightSource&)	= default;
	ILightSource& operator=(ILightSource&&)			= default;

	void InitializeShadowResources(const INITIALIZE_LIGHT_SOURCE_DESC& desc);

	virtual std::string GetLightName() const = 0;
	virtual LightType GetLightType() const = 0;
	//virtual void UpdateViewProjection(const BoundingFrustum& sceneFrustum) = 0;

	ID3D11ShaderResourceView* GetShadowSRV()			const { return m_ShadowSRV.Get(); }
	ID3D11DepthStencilView* GetShadowDSV()				const { return m_ShadowDSVP.Get(); }
	const DirectX::XMMATRIX& GetLightViewProjMatrix()	const { return m_ViewProjMatrix; }


protected:
	// --- Shadow Mapping Resources ---
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			 m_ShadowTexture{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	 m_ShadowDSVP	{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShadowSRV	{ nullptr };

	DirectX::XMMATRIX m_ViewMatrix		= DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_ProjMatrix		= DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_ViewProjMatrix	= DirectX::XMMatrixIdentity();

	UINT m_ShadowWidth{ 0 };
	UINT m_ShadowHeight{ 0 };
};
