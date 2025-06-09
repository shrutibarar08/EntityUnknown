#include "BlobBuilder.h"
#include "ExceptionManager/RenderException.h"


ID3DBlob* BlobBuilder::GetBlob(const BLOB_BUILDER_DESC* desc, UINT flags)
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
    m_Cache[key] = CompileOrLoad(desc, flags);

    return m_Cache[key].Get();
}

Microsoft::WRL::ComPtr<ID3DBlob> BlobBuilder::CompileOrLoad(const BLOB_BUILDER_DESC* desc, UINT flags)
{
    Microsoft::WRL::ComPtr<ID3DBlob> blob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    if (desc->FilePath.ends_with(L".cso"))
    {
        HRESULT hr = D3DReadFileToBlob(desc->FilePath.c_str(), &blob);
        THROW_RENDER_EXCEPTION_IF_FAILED(hr);
    }
    else if (desc->FilePath.ends_with(L".hlsl"))
    {
#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        HRESULT hr = D3DCompileFromFile(
            desc->FilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            desc->EntryPoint.c_str(), desc->Target.c_str(),
            flags, 0, &blob, &errorBlob
        );
        THROW_RENDER_EXCEPTION_IF_FAILED(hr);
    }
    else
    {
        std::string path = std::string(desc->FilePath.begin(), desc->FilePath.end());
        std::string message = "Unsupported shader extension: " + path;
        THROW(message.c_str());
    }

    return blob;
}
