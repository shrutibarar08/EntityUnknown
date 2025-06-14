#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "IModel.h"

typedef struct CUBE_VERTEX_DESC
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TextureCoords;
	DirectX::XMFLOAT3 Normal;
}CUBE_VERTEX_DESC;

class ModelCube final: public IModel
{
	using CubeBuffer = StaticModelBufferSource<CUBE_VERTEX_DESC, 36, uint32_t, 36>;
public:
	ModelCube() = default;
	~ModelCube() override = default;

	ModelCube(const ModelCube&) = default;
	ModelCube(ModelCube&&) = default;
	ModelCube& operator=(const ModelCube&) = default;
	ModelCube& operator=(ModelCube&&) = default;

	bool IsInitialized() const override;

protected:
	bool BuildChild(ID3D11Device* device) override;
	bool RenderChild(ID3D11DeviceContext* deviceContext) override;

private:
	void BuildVertex();
	void BuildIndex();

	bool BuildCubeBuffer(ID3D11Device* device);
	bool BuildShaders(ID3D11Device* device);

private:
	inline static bool m_Initialized{ false };
	inline static std::shared_ptr<CubeBuffer> m_SharedCubeBuffer{ nullptr };
	inline static std::unique_ptr<StaticVBInstance<CubeBuffer>> m_CubeBuffer{ nullptr };
	inline static std::unique_ptr<ShaderResource> m_ShaderResources{ nullptr };

	CUBE_VERTEX_DESC m_Vertices[36]{};
	uint32_t m_Indices[36]{};
};
