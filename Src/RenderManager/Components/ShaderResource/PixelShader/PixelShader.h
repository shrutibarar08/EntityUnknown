#pragma once
#include "RenderManager/Components/ShaderResource/Blob/BlobBuilder.h"


class PixelShader
{
public:
    PixelShader() = default;
    ~PixelShader() = default;

    PixelShader(const PixelShader&) = delete;
    PixelShader(PixelShader&&) = delete;
    PixelShader& operator=(const PixelShader&) = delete;
    PixelShader& operator=(PixelShader&&) = delete;

    static ID3D11PixelShader* Get(ID3D11Device* device, const BLOB_BUILDER_DESC* desc);
    static ID3D11PixelShader* Get(ID3D11Device* device, const std::wstring& pixelShaderPath);
    static ID3D11PixelShader* Get(ID3D11Device* device, const std::string& pixelShaderPath);

private:
    static Microsoft::WRL::ComPtr<ID3D11PixelShader> ConstructPixelShader(ID3D11Device* device, const BLOB_BUILDER_DESC* desc);

private:
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

    inline static std::unordered_map<ShaderKey, Microsoft::WRL::ComPtr<ID3D11PixelShader>, ShaderKeyHash> m_Cache{};
};

