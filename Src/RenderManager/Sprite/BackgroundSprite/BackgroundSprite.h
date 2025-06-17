#pragma once
#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "RenderManager/Sprite/ISprite.h"


class BackgroundSprite final: public ISprite
{
	struct Vertex2D
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Tex;
	};
	using DynamicVBnIB = DynamicModelBufferSource<Vertex2D, uint32_t>;
public:
	BackgroundSprite();
	~BackgroundSprite() override = default;

	BackgroundSprite(const BackgroundSprite&) = delete;
	BackgroundSprite(BackgroundSprite&) = delete;
	BackgroundSprite& operator=(const BackgroundSprite&) = delete;
	BackgroundSprite& operator=(BackgroundSprite&&) = delete;

	void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) override;
	bool IsInitialized() const override;
	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;

private:
	void UpdateVertexBuffer(ID3D11DeviceContext* deviceContext);

protected:
	void BuildShaders(ID3D11Device* device) override;

private:
	//~ Per Instance Shader Data (still using cache tho hehe)
	std::shared_ptr<DynamicVBnIB> m_SharedBitMapBuffer{ nullptr };
	std::unique_ptr<DynamicInstance<DynamicVBnIB>> m_DynamicSpriteBuffer{ nullptr };
	bool m_LocalInitialized{ false };

	std::wstring m_BackgroundVertexShaderPath{ L"Shader/Sprite/ScreenSprite/VertexShader.hlsl" };
	std::wstring m_BackgroundPixelShaderPath{ L"Shader/Sprite/ScreenSprite/PixelShader.hlsl" };

	Vertex2D m_Vertices[6]{};
	uint32_t m_Indices[6]{};
	int m_LastHeight{ -1 }, m_LastWidth{ -1 };
	float m_LastX{ 0.0f }, m_LastY{ 0.0f };
};
