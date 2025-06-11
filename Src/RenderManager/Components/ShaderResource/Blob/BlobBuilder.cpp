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

    std::string filePathStr(desc->FilePath.begin(), desc->FilePath.end());

    if (desc->FilePath.ends_with(L".cso"))
    {
        HRESULT hr = D3DReadFileToBlob(desc->FilePath.c_str(), &blob);
        if (FAILED(hr))
        {
            LOG_ERROR("[BlobBuilder] Failed to load compiled shader (.cso) file: " + filePathStr);
            LOG_ERROR("HRESULT: " + std::to_string(hr));
            THROW_RENDER_EXCEPTION_IF_FAILED(hr);
        }
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

        if (FAILED(hr))
        {
            LOG_ERROR("[BlobBuilder] Failed to compile shader: " + filePathStr);
            LOG_ERROR("Entry Point: " + desc->EntryPoint);
            LOG_ERROR("Target Profile: " + desc->Target);
            LOG_ERROR("HRESULT: " + std::to_string(hr));

            if (errorBlob)
            {
                std::string errorMsg(
                    static_cast<const char*>(errorBlob->GetBufferPointer()),
                    errorBlob->GetBufferSize()
                );
                LOG_ERROR("Compiler Output:\n" + errorMsg);
            }
            else
            {
                LOG_ERROR("No compiler output available.");
            }

            THROW_RENDER_EXCEPTION_IF_FAILED(hr);
        }
    }
    else
    {
        std::string message = "Unsupported shader extension: " + filePathStr;
        THROW(message.c_str());
    }

    return blob;
}
