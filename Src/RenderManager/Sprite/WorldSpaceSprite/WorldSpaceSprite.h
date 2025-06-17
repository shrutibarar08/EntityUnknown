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

	WorldSpaceSprite(const WorldSpaceSprite&) = delete;
	WorldSpaceSprite(WorldSpaceSprite&&) = delete;
	WorldSpaceSprite& operator=(const WorldSpaceSprite&) = delete;
	WorldSpaceSprite& operator=(WorldSpaceSprite&&) = delete;

	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;
	void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) override;
	bool IsInitialized() const override;

private:
	void BuildVertexBuffer();
	void BuildIndexBuffer();

protected:
	void BuildShaders(ID3D11Device* device) override;

private:
	bool m_LocalInitialized{ false };
	std::wstring m_WorldSpaceSpriteVS{ L"Shader/Sprite/SpaceSprite/VertexShader.hlsl" };
	std::wstring m_WorldSpaceSpritePS{ L"Shader/Sprite/SpaceSprite/PixelShader.hlsl" };

	std::shared_ptr<StaticVBData> m_SharedVBnIB{ nullptr };
	std::unique_ptr<StaticVBInstance<StaticVBData>> m_StaticSpriteVBnIB{ nullptr };

	Vertex2D m_Vertices[6]{};
	uint32_t m_Indices[6]{};
};
