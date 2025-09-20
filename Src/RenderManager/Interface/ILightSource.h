#pragma once
#include "SystemManager/PrimaryID.h"
#include <DirectXMath.h>
#include <string>
#include <d3d11.h>
#include <wrl/client.h>

#include "RenderManager/Frustum/Frustum.h"
#include "Utils/SweetLoader/SweetLoader.h"

#include <nlohmann/json.hpp>

class LevelEditorContext;

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

static inline DirectX::XMVECTOR SafeFromToQuat(DirectX::XMVECTOR from, DirectX::XMVECTOR to)
{
	float fromLen2 = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(from));
	if (fromLen2 < 1e-12f) return DirectX::XMQuaternionIdentity();
	from = DirectX::XMVectorScale(from, 1.0f / sqrtf(fromLen2));

	float toLen2 = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(to));
	if (toLen2 < 1e-12f) return DirectX::XMQuaternionIdentity();
	to = DirectX::XMVectorScale(to, 1.0f / sqrtf(toLen2));

	float d = DirectX::XMVectorGetX(DirectX::XMVector3Dot(from, to));
	d = (d > 1.0f ? 1.0f : d);
	d = (d < -1.0f ? -1.0f : d);

	if (d > 0.999999f) return DirectX::XMQuaternionIdentity();

	if (d < -0.999999f)
	{
		DirectX::XMVECTOR axis = DirectX::XMVector3Cross(from, DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f));
		if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(axis)) < 1e-8f)
			axis = DirectX::XMVector3Cross(from, DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f));
		axis = DirectX::XMVector3Normalize(axis);
		return DirectX::XMQuaternionRotationAxis(axis, DirectX::XM_PI);
	}

	DirectX::XMVECTOR axis = DirectX::XMVector3Cross(from, to);
	float axisLen2 = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(axis));
	if (axisLen2 < 1e-16f) return DirectX::XMQuaternionIdentity(); // super-degenerate fallback
	axis = DirectX::XMVectorScale(axis, 1.0f / sqrtf(axisLen2));

	float angle = acosf(d);
	return DirectX::XMQuaternionRotationAxis(axis, angle);
}

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
	virtual std::string GetLightTypeToString() const = 0;
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

	std::string GetLightName() const { return m_LightName; }
	void SetLightName(const std::string& name) { m_LightName = name; }

	virtual void RenderControlUI(LevelEditorContext* context)  = 0;
	virtual void LoadLightSaveData(const nlohmann::json& data) = 0;
	virtual nlohmann::json GetLightSaveData() const			   = 0;

	void SetTypeName(const std::string& type) { m_LightType = type; }
	std::string GetTypeName() const { return m_LightType; }

protected:
	static void PrintLightMatrix(const DirectX::XMMATRIX& mat);

protected:
	bool m_bInitialized{ true };
	std::string m_LightName{ "Not Set!" };

	// --- Shadow Mapping Resources ---
	ID3D11DepthStencilView*	 m_ShadowDSV { nullptr };

	DirectX::XMMATRIX m_ViewMatrix		= DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_ProjMatrix		= DirectX::XMMatrixIdentity();

	UINT m_ShadowWidth{ 2048 };
	UINT m_ShadowHeight{ 2048 };

private:
	std::string m_LightType{ "Undefined Type" };
};
