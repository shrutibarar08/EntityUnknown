#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <unordered_map>
#include <string>

typedef struct BLOB_BUILDER_DESC
{
    std::wstring FilePath;
    std::string EntryPoint;
    std::string Target;

    bool IsEmpty() const
    {
        return FilePath.empty() || EntryPoint.empty() || Target.empty();
    }

}BLOB_BUILDER_DESC;

class BlobBuilder
{
public:
    BlobBuilder() = default;
    static ID3DBlob* GetBlob(const BLOB_BUILDER_DESC* desc, UINT flags = D3DCOMPILE_ENABLE_STRICTNESS);
private:
    static Microsoft::WRL::ComPtr<ID3DBlob> CompileOrLoad(const BLOB_BUILDER_DESC* desc, UINT flags = D3DCOMPILE_ENABLE_STRICTNESS);

    struct ShaderKey
    {
        std::wstring FilePath;
        std::string EntryPoint;
        std::string Target;

        bool operator==(const ShaderKey& other) const
        {
            return FilePath == other.FilePath &&
                EntryPoint == other.EntryPoint &&
                Target == other.Target;
        }
    };

    struct ShaderKeyHash
    {
        std::size_t operator()(const ShaderKey& k) const
        {
            return std::hash<std::wstring>()(k.FilePath) ^
                std::hash<std::string>()(k.EntryPoint) ^
                std::hash<std::string>()(k.Target);
        }
    };

    inline static std::unordered_map<ShaderKey, Microsoft::WRL::ComPtr<ID3DBlob>, ShaderKeyHash> m_Cache;
};
