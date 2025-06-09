#pragma once
#include <d3d11.h>
#include <wrl/client.h>


class IModel
{
public:
	IModel() = default;
	~IModel() = default;

	IModel(const IModel&) = default;
	IModel(IModel&&) = default;
	IModel& operator=(const IModel&) = default;
	IModel& operator=(IModel&&) = default;

	bool Init(ID3D11Device* device);
	virtual void Shutdown() = 0;
	virtual void Render(ID3D11DeviceContext* deviceContext) = 0;
	virtual UINT GetIndexCount() = 0;

protected:
	virtual Microsoft::WRL::ComPtr<ID3D11Buffer> BuildVertexBuffer() = 0;
	virtual Microsoft::WRL::ComPtr<ID3D11Buffer> BuildIndexBuffer() = 0;

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
};
