#pragma once
#include "RenderManager/Components/ShaderResource/Blob/BlobBuilder.h"


class VertexShader
{
public:
	VertexShader() = default;
	~VertexShader() = default;

	VertexShader(const VertexShader&) = delete;
	VertexShader(VertexShader&&) = delete;
	VertexShader& operator=(const VertexShader&) = delete;
	VertexShader& operator=(VertexShader&&) = delete;

	static ID3D11VertexShader* Get(ID3D11Device* device, const BLOB_BUILDER_DESC* desc);
	static ID3D11VertexShader* Get(ID3D11Device* device, const std::wstring& vertexShaderPath);
	static ID3D11VertexShader* Get(ID3D11Device* device, const std::string& vertexShaderPath);

private:
    static Microsoft::WRL::ComPtr<ID3D11VertexShader> ConstructVertexShader(ID3D11Device* device, const BLOB_BUILDER_DESC* desc);

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

    inline static std::unordered_map<ShaderKey, Microsoft::WRL::ComPtr<ID3D11VertexShader>, ShaderKeyHash> m_Cache{};
};

