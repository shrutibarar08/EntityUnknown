#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "RenderManager/Model/IModel.h"

typedef struct CUBE_VERTEX_DESC
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TextureCoords;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 BiNormal;
}CUBE_VERTEX_DESC;

class ModelCube final: public IModel
{
	using CubeBuffer = StaticModelBufferSource<CUBE_VERTEX_DESC, uint32_t>;
public:
	ModelCube() = default;
	~ModelCube() override = default;

	ModelCube(const ModelCube&)				= delete;
	ModelCube(ModelCube&&)					= delete;
	ModelCube& operator=(const ModelCube&)	= delete;
	ModelCube& operator=(ModelCube&&)		= delete;

	bool IsInitialized() const override;

	void RenderDebug(ID3D11DeviceContext* deviceContext, const VERTEX_BUFFER_METADATA_GPU& gpuData) const;

protected:
	bool BuildChild(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	bool RenderChild(ID3D11DeviceContext* deviceContext) override;
	void BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	void RenderGeometry(ID3D11DeviceContext* deviceContext) override;

private:
	void BuildVertex();
	void BuildIndex();

	bool BuildCubeBuffer(ID3D11Device* device);

public:
	void ResetInitialization() override;

private:
	bool m_Initialized{ false };
	std::shared_ptr<CubeBuffer> m_SharedCubeBuffer{ nullptr };
	std::unique_ptr<StaticVBInstance<CubeBuffer>> m_CubeBuffer{ nullptr };

	std::vector<CUBE_VERTEX_DESC> m_Vertices{};
	std::vector<uint32_t> m_Indices{};
};
