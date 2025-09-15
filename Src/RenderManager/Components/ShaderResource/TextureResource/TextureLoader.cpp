#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "TextureLoader.h"
#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include "ExceptionManager/IException.h"
#include "ExceptionManager/RenderException.h"
#include "RenderManager/RenderQueue/RenderQueue.h"

namespace
{
    bool LoadWithStb_RGBA8(
        ID3D11Device* device,
        ID3D11DeviceContext* ctx,
        const std::string& path,
        TextureLoader::TextureResource& out)
    {
        int w = 0, h = 0, comp = 0;

        unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &comp, STBI_rgb_alpha);
        if (!pixels)
        {
            LOG_ERROR(std::string("stb_image: failed to load '") + path + "' : " + stbi_failure_reason());
            return false;
        }

        // Describe a GPU texture we can GenerateMips on
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = static_cast<UINT>(w);
        desc.Height = static_cast<UINT>(h);
        desc.MipLevels = 0;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = device->CreateTexture2D(&desc, nullptr, tex.GetAddressOf());
        if (FAILED(hr))
        {
            stbi_image_free(pixels);
            LOG_ERROR("CreateTexture2D failed for '" + path + "'");
            return false;
        }

        // Upload base level
        const UINT rowPitch = static_cast<UINT>(w) * 4; // RGBA8
        ctx->UpdateSubresource(tex.Get(), 0, nullptr, pixels, rowPitch, 0);

        // Create SRV covering all mips
        D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
        srvd.Format = desc.Format;
        srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvd.Texture2D.MostDetailedMip = 0;
        srvd.Texture2D.MipLevels = -1; // all mips

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        hr = device->CreateShaderResourceView(tex.Get(), &srvd, srv.GetAddressOf());

        // free CPU data ASAP
        stbi_image_free(pixels);

        if (FAILED(hr))
        {
            LOG_ERROR("CreateShaderResourceView failed for '" + path + "'");
            return false;
        }

        ctx->GenerateMips(srv.Get());

        out.Texture            = tex;
        out.ShaderResourceView = srv;
        out.Width              = w;
        out.Height             = h;

        return true;
    }

    std::string GetLowerExt(const std::string& path)
    {
        const auto dot = path.find_last_of('.');
        if (dot == std::string::npos) return {};
        std::string ext = path.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
}

TEXTURE_RESOURCE TextureLoader::GetTexture(const std::string& path)
{
    if (!RenderQueue::Get()) return {};
    if (!RenderQueue::Get()->GetDevice() || !RenderQueue::Get()->GetDeviceContext()) return {};

    return GetTexture(RenderQueue::Get()->GetDevice(), RenderQueue::Get()->GetDeviceContext(), path);
}

TEXTURE_RESOURCE TextureLoader::GetTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& path)
{
    if (path.empty()) return {};

    if (!m_Cache.contains(path))
    {
        LOG_INFO("Cache miss for: " + path + " — creating texture...");
        if (!BuildTexture(device, deviceContext, path))
        {
            LOG_ERROR("Failed to build texture: " + path);
            return {};
        }
        LOG_SUCCESS("Texture created: " + path);
    }

    const auto& cached = m_Cache[path];

    TEXTURE_RESOURCE out{};
    out.ShaderResourceView = cached.ShaderResourceView.Get();
    out.Texture = cached.Texture.Get();
    out.Width = cached.Width;
    out.Height = cached.Height;
    out.TexturePath = path;
    return out;
}

bool TextureLoader::BuildTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& path)
{
    const std::string ext = GetLowerExt(path);
    if (ext.empty())
    {
        LOG_ERROR("No file extension: " + path);
        return false;
    }

    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga" ||
        ext == "psd" || ext == "gif" || ext == "hdr" || ext == "pic" || ext == "pnm" || ext == "ppm" || ext == "pgm")
    {
        TextureResource res{};
        if (!LoadWithStb_RGBA8(device, deviceContext, path, res))
            return false;

        m_Cache[path] = std::move(res);
        return true;
    }
    else if (ext == "dds")
    {
        LOG_ERROR("DDS not handled by stb_image. Use a DDS-specific path.");
        return false;
    }
    else
    {
        LOG_ERROR("Unsupported image extension: " + ext + " (file: " + path + ")");
        return false;
    }
}

bool TextureLoader::LoadTarga32Bit(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& path)
{
    TextureResource res{};
    if (!LoadWithStb_RGBA8(device, deviceContext, path, res))
        return false;

    m_Cache[path] = std::move(res);
    LOG_SUCCESS("Loaded TGA via stb_image: " + path);
    return true;
}
