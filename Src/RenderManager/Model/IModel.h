#pragma once
#include "RenderManager/IRender.h"
#include "RenderManager/Components/ShaderResource/LightBuffer/LightBuffer.h"
#include "RenderManager/Light/DirectionalLight.h"


typedef struct TRANSFORM_3D_GPU
{
	DirectX::XMMATRIX NormalMatrix;
}TRANSFORM_3D_GPU;


class IModel: public IRender
{
	using DirectionalBufferConfig = LightBuffer<DIRECTIONAL_Light_DATA, 10, false>;
public:
	IModel()							= default;
	virtual ~IModel() override			= default;
	IModel(const IModel&)				= default;
	IModel(IModel&&)					= default;
	IModel& operator=(const IModel&)	= default;
	IModel& operator=(IModel&&)			= default;

	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;

	void AddLight(ILightAnyType* lightSource);
	void RemoveLight(ILightAnyType* lightSource);
	void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) override;

protected:
	virtual bool BuildChild(ID3D11Device* device) = 0;
	virtual bool RenderChild(ID3D11DeviceContext* deviceContext) = 0;

protected:
	//~ Light Buffer and Manager
	LightBufferManager m_LightBufferManager{};

	inline static bool m_bWorldMatrixInitialized{ false };
	inline static std::unique_ptr<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>> m_WorldMatrixConstantBuffer{ nullptr };
	inline static bool m_bModelCommonDataInitialized{ false };
	inline static bool m_LightMetaUpdated{ false };
	inline static std::unique_ptr<ConstantBuffer<PIXEL_LIGHT_META_GPU>> m_LightMetaCB{ nullptr };

	//~ 3D Model Specific Constant Buffer
	inline static std::unique_ptr<ConstantBuffer<TRANSFORM_3D_GPU>> m_3DModelConstantBuffer{ nullptr };
	TRANSFORM_3D_GPU m_3DTransformData{};
};
