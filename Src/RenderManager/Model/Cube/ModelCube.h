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
	using CubeBuffer = StaticModelBufferSource<CUBE_VERTEX_DESC, 24, uint32_t, 36>;
public:
	ModelCube() = default;
	~ModelCube() override = default;

	ModelCube(const ModelCube&)				= delete;
	ModelCube(ModelCube&&)					= delete;
	ModelCube& operator=(const ModelCube&)	= delete;
	ModelCube& operator=(ModelCube&&)		= delete;

	bool IsInitialized() const override;
	void SetTextureMultiplier(int valueX, int valueY);

protected:
	bool BuildChild(ID3D11Device* device) override;
	bool RenderChild(ID3D11DeviceContext* deviceContext) override;
	void BuildShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

private:
	void BuildVertex();
	void BuildIndex();

	bool BuildCubeBuffer(ID3D11Device* device);
private:
	int m_TextureMultiplierX{ 1 };
	int m_TextureMultiplierY{ 1 };

	bool m_Initialized{ false };
	std::shared_ptr<CubeBuffer> m_SharedCubeBuffer{ nullptr };
	std::unique_ptr<StaticVBInstance<CubeBuffer>> m_CubeBuffer{ nullptr };

	CUBE_VERTEX_DESC m_Vertices[24]{};
	uint32_t m_Indices[36]{};
};
