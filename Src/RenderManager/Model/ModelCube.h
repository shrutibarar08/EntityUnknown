#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include "IModel.h"
#include "RenderManager/Components/ConstantBuffer.h"
#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"

typedef struct CUBE_VERTEX_DESC
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT2 TextureCoords;
}CUBE_VERTEX_DESC;

class ModelCube final: public IModel
{
	using CubeBuffer = StaticModelBufferSource<CUBE_VERTEX_DESC, 24, uint32_t, 36>;
public:
	ModelCube() = default;
	~ModelCube() override = default;

	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;
	void UpdateTransformation(const CAMERA_MATRIX_DESC* cameraInfo) override;
	bool IsInitialized() const override;

private:
	void BuildVertex();
	void BuildIndex();

private:
	inline static bool m_Initialized{ false };
	inline static std::shared_ptr<CubeBuffer> m_SharedCubeBuffer{ nullptr };
	inline static std::unique_ptr<ModelInstance<CubeBuffer>> m_CubeBuffer{ nullptr };
	inline static std::unique_ptr<ShaderResource> m_ShaderResources{ nullptr };
	inline static std::unique_ptr<IConstantBuffer> m_VertexConstantBuffer{ nullptr };

	CUBE_VERTEX_DESC m_Vertices[24];
	uint32_t m_Indices[36];
};
