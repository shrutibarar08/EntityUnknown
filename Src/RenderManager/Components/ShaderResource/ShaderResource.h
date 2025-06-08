#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>
#include <wrl/client.h>


typedef struct SHADER_CONSTANT_BUFFER_DESC
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
}SHADER_CONSTANT_BUFFER_DESC;

class ShaderResource
{
public:
	ShaderResource() = default;
	virtual ~ShaderResource() = default;

	ShaderResource(const ShaderResource&) = delete;
	ShaderResource(ShaderResource&&) = delete;
	ShaderResource& operator=(const ShaderResource&) = delete;
	ShaderResource& operator=(ShaderResource&&) = delete;

	void SetVertexShaderPath(const std::string& path);
	void SetVertexShaderPath(const std::wstring& path);

	void SetPixelShaderPath(const std::string& path);
	void SetPixelShaderPath(const std::wstring& path);

	bool Initialize(ID3D11Device* device, HWND handle);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context, int indexCount, const SHADER_CONSTANT_BUFFER_DESC* desc);

protected:
	bool InitializeShader(ID3D11Device* device, HWND handle);
	void ShutdownShader();

	bool SetShaderParameters(ID3D11DeviceContext* context, const SHADER_CONSTANT_BUFFER_DESC* desc);
	void RenderShader(ID3D11DeviceContext* context, int indexCount);

private:
	std::wstring m_VertexShaderPath;
	std::wstring m_PixelShaderPath;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_Layout{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexConstantBuffer{ nullptr };
};
