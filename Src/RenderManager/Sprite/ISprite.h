#pragma once
#include "RenderManager/Interface/IRender.h"
#include "RenderManager/Model/Cube/ModelCube.h"

class ISprite : public IRender
{
public:
	ISprite() = default;
	virtual ~ISprite() override = default;

	ISprite(const ISprite&) = delete;
	ISprite(ISprite&&) = delete;
	ISprite& operator=(const ISprite&) = delete;
	ISprite& operator=(ISprite&&) = delete;

	// Setters
	void SetLeftPercent(float percent);
	void SetRightPercent(float percent);
	void SetTopPercent(float percent);
	void SetDownPercent(float percent);

	// Getters
	float GetLeftPercent() const;
	float GetRightPercent() const;
	float GetTopPercent() const;
	float GetDownPercent() const;

	// Combined setter/getter
	void SetEdgePercents(float left, float right, float top, float down);
	void GetEdgePercents(float& left, float& right, float& top, float& down) const;
	bool Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

protected:
	float m_LeftPercent{ 0.25f };
	float m_RightPercent{ 0.25f };
	float m_TopPercent{ 0.25f };
	float m_DownPercent{ 0.25f };

	inline static std::unique_ptr<ModelCube> m_DebugCube{ nullptr };
};
