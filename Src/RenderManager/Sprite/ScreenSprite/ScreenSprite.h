#pragma once
#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "RenderManager/Sprite/ISprite.h"


class ScreenSprite final: public ISprite
{
	struct Vertex2D
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Tex;
	};
	using DynamicVBnIB = DynamicModelBufferSource<Vertex2D, uint32_t>;
public:
	ScreenSprite();
	~ScreenSprite() override = default;

	void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) override;
	bool IsInitialized() const override;
	void UpdateTextureResource(const TEXTURE_RESOURCE& resource) override;
	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;

private:
	void UpdateVertexBuffer(ID3D11DeviceContext* deviceContext);

private:
	//~ Per Instance Shader Data (still using cache tho hehe)
	std::unique_ptr<ShaderResource> m_ShaderResources{ nullptr };
	std::shared_ptr<DynamicVBnIB> m_SharedBitMapBuffer{ nullptr };
	std::unique_ptr<DynamicInstance<DynamicVBnIB>> m_DynamicSpriteBuffer{ nullptr };
	bool m_LocalInitialized{ false };

	Vertex2D m_Vertices[6]{};
	uint32_t m_Indices[6]{};
	int m_LastHeight{ -1 }, m_LastWidth{ -1 };
};
