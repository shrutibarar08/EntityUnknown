#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include "RenderManager/Components/ConstantBuffer.h"
#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"

typedef struct CUBE_VERTEX_DESC
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT4 Color;
}CUBE_VERTEX_DESC;

typedef struct WORLD_TRANSFORM
{
	DirectX::XMMATRIX TransformationMatrix;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
}WORLD_TRANSFORM;

class ModelCube
{
	using CubeBuffer = StaticModelBufferSource<CUBE_VERTEX_DESC, 8, uint32_t, 36>;
public:
	ModelCube() = default;
	~ModelCube() = default;

	bool Build(ID3D11Device* device);
	bool Render(ID3D11DeviceContext* deviceContext) const;

	WORLD_TRANSFORM& GetWorldTransform();

private:
	void BuildVertex();
	void BuildIndex();

private:
	inline static bool m_Initialized{ false };
	inline static std::shared_ptr<CubeBuffer> m_SharedCubeBuffer{ nullptr };
	inline static std::unique_ptr<ModelInstance<CubeBuffer>> m_CubeBuffer{ nullptr };
	inline static std::unique_ptr<ShaderResource> m_ShaderResources{ nullptr };
	inline static std::unique_ptr<IConstantBuffer> m_VertexConstantBuffer{ nullptr };

	CUBE_VERTEX_DESC m_Vertices[8];
	uint32_t m_Indices[36];
	WORLD_TRANSFORM m_WorldTransform;
};
