#include "PixelShader.h"

#include "ExceptionManager/RenderException.h"

ID3D11PixelShader* PixelShader::Get(ID3D11Device* device, const BLOB_BUILDER_DESC* desc)
{
    ShaderKey key
    {
        desc->FilePath,
        desc->EntryPoint,
        desc->Target
    };

    if (m_Cache.contains(key))
    {
        return m_Cache[key].Get();
    }
    m_Cache[key] = ConstructPixelShader(device, desc);
    return m_Cache[key].Get();
}

ID3D11PixelShader* PixelShader::Get(ID3D11Device* device, const std::wstring& pixelShaderPath)
{
	BLOB_BUILDER_DESC desc{};
	desc.EntryPoint = "main";
	desc.FilePath = pixelShaderPath;
	desc.Target = "ps_5_0";

	return Get(device, &desc);
}

ID3D11PixelShader* PixelShader::Get(ID3D11Device* device, const std::string& pixelShaderPath)
{
	std::wstring wPath = std::wstring(pixelShaderPath.begin(), pixelShaderPath.end());
	return Get(device, wPath);
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader::ConstructPixelShader(ID3D11Device* device,
	const BLOB_BUILDER_DESC* desc)
{
    ID3DBlob* blob = BlobBuilder::GetBlob(desc);
    Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;

    const HRESULT result = device->CreatePixelShader(
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        nullptr, &shader);

    THROW_RENDER_EXCEPTION_IF_FAILED(result);

    return shader;
}
