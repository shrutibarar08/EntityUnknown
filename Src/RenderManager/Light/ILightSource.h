#pragma once
#include "SystemManager/PrimaryID.h"
#include <DirectXMath.h>
#include <string>
#include <d3d11.h>
#include <wrl/client.h>

#include "RenderManager/Frustum/Frustum.h"

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

class ILightSource: public PrimaryID
{
public:
	ILightSource()									= default;
	virtual ~ILightSource()							= default;
	ILightSource(const ILightSource&)				= default;
	ILightSource(ILightSource&&)					= default;
	ILightSource& operator=(const ILightSource&)	= default;
	ILightSource& operator=(ILightSource&&)			= default;

	virtual DirectX::XMFLOAT3 GetLightPosition() const = 0;
	virtual std::string GetLightName() const = 0;
	virtual LightType GetLightType() const = 0;
	virtual void UpdateProjectionMatrix(const Frustum& sceneFrustum) = 0;
	void ComputeViewMatrix(const DirectX::XMVECTOR& targetPosition);

	ID3D11DepthStencilView* GetShadowDSV() const { return m_ShadowDSV; }
	bool IsShadowDSVAssigned() const { return m_ShadowDSV != nullptr; }
	void SetShadowDSV(ID3D11DepthStencilView* shadow) { m_ShadowDSV = shadow; }

	const DirectX::XMMATRIX& GetLightViewProjMatrix()	const { return m_ViewMatrix * m_ProjMatrix; }
	DirectX::XMMATRIX GetViewMatrix() const { return m_ViewMatrix; }
	DirectX::XMMATRIX GetProjectionMatrix() const { return m_ProjMatrix; }
	DirectX::XMINT2 GetShadowResolution() const;

	bool IsInitialized() const { return m_bInitialized; }

protected:
	static void PrintLightMatrix(const DirectX::XMMATRIX& mat);

protected:
	bool m_bInitialized{ true };

	// --- Shadow Mapping Resources ---
	ID3D11DepthStencilView*	 m_ShadowDSV { nullptr };

	DirectX::XMMATRIX m_ViewMatrix		= DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_ProjMatrix		= DirectX::XMMatrixIdentity();

	UINT m_ShadowWidth{ 2048 };
	UINT m_ShadowHeight{ 2048 };
};
