#include "VertexShader.h"

#include "ExceptionManager/IException.h"
#include "ExceptionManager/RenderException.h"

ID3D11VertexShader* VertexShader::Get(ID3D11Device* device, const BLOB_BUILDER_DESC* desc)
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
    m_Cache[key] = ConstructVertexShader(device, desc);
    return m_Cache[key].Get();
}

ID3D11VertexShader* VertexShader::Get(ID3D11Device* device, const std::wstring& vertexShaderPath)
{
    BLOB_BUILDER_DESC desc{};
    desc.EntryPoint = "main";
    desc.FilePath = vertexShaderPath;
    desc.Target = "vs_5_0";

    return Get(device, &desc);
}

ID3D11VertexShader* VertexShader::Get(ID3D11Device* device, const std::string& vertexShaderPath)
{
    std::wstring wPath = std::wstring(vertexShaderPath.begin(), vertexShaderPath.end());
    return Get(device, wPath);
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader::ConstructVertexShader(ID3D11Device* device, const BLOB_BUILDER_DESC* desc)
{
    ID3DBlob* blob = BlobBuilder::GetBlob(desc);
    Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;

    const HRESULT result = device->CreateVertexShader(
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        nullptr, &shader);

    THROW_RENDER_EXCEPTION_IF_FAILED(result);

    return shader;
}
