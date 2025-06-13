#pragma once
#include <DirectXMath.h>

#include "RenderManager/IRender.h"
#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"


class IBitmap final: public IRender
{
	struct Vertex2D
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Tex;
	};
	using BitMapBuffer = DynamicModelBufferSource<Vertex2D, uint32_t>;

public:
	IBitmap() = default;
	virtual ~IBitmap() override = default;

	bool IsInitialized() const override;

	bool Build(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext) override;

	void SetTexture(const std::string& texture);
	void SetWorldMatrixData(const CAMERA_INFORMATION_DESC& cameraInfo) override;

private:
	void UpdateVertexBuffer(ID3D11DeviceContext* deviceContext);

private:
	std::unique_ptr<ShaderResource> m_ShaderResources{ nullptr };
	std::string m_TextureToAdd{};
	bool m_LocalInitialized{ false };

	inline static bool m_bStaticInitialized{ false };
	inline static bool m_bDynamicBufferInitialized{ false };
	inline static std::unique_ptr<ConstantBuffer<WORLD_TRANSFORM_GPU_DESC>> m_BitmapWorldMatrixCB{ nullptr };
	std::shared_ptr<BitMapBuffer> m_SharedBitMapBuffer{ nullptr };
	std::unique_ptr<BitmapInstance<BitMapBuffer>> m_BitMapBuffer{ nullptr };
	Vertex2D m_Vertices[6];
	uint32_t m_Indices[6];

	int m_LastHeight{ -1 }, m_LastWidth{ -1 };
	int m_LazyWait{ 100 };
};
