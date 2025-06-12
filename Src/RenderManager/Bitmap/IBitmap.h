#pragma once
#include <DirectXMath.h>

#include "RenderManager/Components/ConstantBuffer.h"
#include "RenderManager/Components/ModelBuffer.h"
#include "RenderManager/Components/ShaderResource/ShaderResource.h"
#include "SystemManager/PrimaryID.h"


typedef struct CAMERA_2D_MATRIX_DESC
{
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
}CAMERA_2D_MATRIX_DESC;

typedef struct WORLD_2D_TRANSFORM
{
	DirectX::XMMATRIX WorldMatrix;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
}WORLD_2D_TRANSFORM;

class IBitmap: public PrimaryID
{
	struct Vertex2D
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Tex;
	};
	using BitMapBuffer = DynamicModelBufferSource<Vertex2D, uint32_t>;
public:
	IBitmap() = default;
	virtual ~IBitmap() = default;

	//~ Configs
	void SetScreenSize(int x, int y);
	void SetScale(float x, float y, float z);
	void SetRenderPosition(float x, float y);

	bool IsInitialized() const;

	bool Build(ID3D11Device* device);
	bool Render(ID3D11DeviceContext* deviceContext);
	void UpdateTransformation(const CAMERA_2D_MATRIX_DESC& cameraInfo);
	void SetTexture(const std::string& texture);

private:
	void UpdateVertexBuffer(ID3D11DeviceContext* deviceContext);

private:
	DirectX::XMFLOAT3 m_Scale{ 1.f, 1.f, 1.f };
	std::string m_TextureToAdd{};
	bool m_Initialized{ false };
	std::shared_ptr<BitMapBuffer> m_SharedBitMapBuffer{ nullptr };
	std::unique_ptr<BitmapInstance<BitMapBuffer>> m_BitMapBuffer{ nullptr };
	std::unique_ptr<ShaderResource> m_ShaderResources{ nullptr };
	inline static std::unique_ptr<IConstantBuffer> m_VertexConstantBuffer{ nullptr };
	inline static std::unique_ptr<IConstantBuffer> m_PixelConstantBuffer{ nullptr };

	WORLD_2D_TRANSFORM m_WorldTransform{};

	int m_ScreenWidth{ 1280 }, m_ScreenHeight{ 720 };
	float m_PosX = 0.0f, m_PosY = 0.0f, m_PrevX = -2, m_PrevY = -2;
};
