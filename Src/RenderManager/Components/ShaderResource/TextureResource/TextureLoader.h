#pragma once
#include <d3d11.h>
#include <string>
#include <unordered_map>
#include <wrl/client.h>

typedef struct TEXTURE_RESOURCE
{
	ID3D11ShaderResourceView* ShaderResourceView;
	ID3D11Texture2D* Texture;

	int Height;
	int Width;

	bool IsInitialized() const
	{
		return ShaderResourceView != nullptr && Texture != nullptr;
	}

}TEXTURE_RESOURCE;

class TextureLoader
{
public:
	TextureLoader() = default;
	~TextureLoader() = default;

	TextureLoader(const TextureLoader&) = delete;
	TextureLoader(TextureLoader&&) = delete;
	TextureLoader& operator=(const TextureLoader&) = delete;
	TextureLoader& operator=(TextureLoader&&) = delete;

	static TEXTURE_RESOURCE GetTexture(ID3D11Device* device, const std::string& path);

private:
	static bool BuildTexture(ID3D11Device* device, const std::string& path);
	static bool LoadTarga32Bit(ID3D11Device* device, const std::string& path);

private:
    struct TextureResource
    {
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;

		int Height;
		int Width;
    };

    inline static std::unordered_map<std::string, TextureResource> m_Cache;
};
