#pragma once
#include "RenderManager/IRender.h"

class IModel : public IRender
{
public:
	IModel() = default;
	virtual ~IModel() override		= default;
	IModel(const IModel&)			= delete;
	IModel(IModel&&)				= delete;
	IModel& operator=(const IModel&)= delete;
	IModel& operator=(IModel&&)		= delete;

	bool Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;

	void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) override;

protected:
	virtual bool BuildChild(ID3D11Device* device) = 0;
	virtual bool RenderChild(ID3D11DeviceContext* deviceContext) = 0;
};
