#pragma once
#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "RenderManager/Sprite/ISprite.h"


class WorldSpaceSprite final: public ISprite
{
	struct Vertex2D
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Tex;
	};

	using StaticVBData = StaticModelBufferSource<Vertex2D, 6, uint32_t, 6>;
public:
	WorldSpaceSprite();
	~WorldSpaceSprite() override = default;

	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;
	void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) override;
	bool IsInitialized() const override;
	void UpdateTextureResource(const TEXTURE_RESOURCE& resource) override;
private:
	void BuildVertexBuffer();
	void BuildIndexBuffer();

private:
	bool m_LocalInitialized{ false };

	//~ Common Vertex and Index Buffer
	TEXTURE_RESOURCE m_TextureResource{};
	inline static std::shared_ptr<StaticVBData> m_SharedVBnIB{ nullptr };
	inline static std::unique_ptr<StaticVBInstance<StaticVBData>> m_StaticSpriteVBnIB{ nullptr };
	Vertex2D m_Vertices[6]{};
	uint32_t m_Indices[6]{};

	//~ Shader Resources
	std::unique_ptr<ShaderResource> m_ShaderResource{ nullptr };
};
