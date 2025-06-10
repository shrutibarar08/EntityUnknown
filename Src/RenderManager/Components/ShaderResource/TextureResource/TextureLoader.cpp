#include "TextureLoader.h"
#include <fstream>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <cstring>

#include "ExceptionManager/IException.h"
#include "ExceptionManager/RenderException.h"

TEXTURE_RESOURCE TextureLoader::GetTexture(ID3D11Device* device, const std::string& path)
{
    // If texture is already in the cache, return it
    if (!m_Cache.contains(path))
    {
        LOG_INFO("Cache for path: " + path + " not found creating it...");
        if (!BuildTexture(device, path)) return {};
        LOG_SUCCESS("Creation Complete!");
    }

    // Return the freshly loaded texture
    TEXTURE_RESOURCE resource{};
    resource.ShaderResourceView = m_Cache[path].ShaderResourceView.Get();
    resource.Texture = m_Cache[path].Texture.Get();
    return resource;
}

bool TextureLoader::BuildTexture(ID3D11Device* device, const std::string& path)
{
    // Convert extension to lowercase for safety
    std::string extension;
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        extension = path.substr(dotPos + 1);
    }
    else
    {
        std::string message = "The Path does not consist of any path: " + path;
        THROW(message.c_str());
	    return false; // No extension found
    }

    // Lowercase extension
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == "tga")
    {
        return LoadTarga32Bit(device, path);
    }
    else if (extension == "dds")
    {
        // return LoadDDS(device, path); // to be implemented
        return false;
    }
    else if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "bmp")
    {
        // return LoadWIC(device, path); // to be implemented
        return false;
    }

    // Unsupported format
    std::string message = "The Extension is not supported: " + path;
    THROW(message.c_str());
    return false;
}

bool TextureLoader::LoadTarga32Bit(ID3D11Device* device, const std::string& path)
{
    // Open file
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        std::string message = "Failed to open TGA file: " + path;
        LOG_ERROR(message);
        return false;
    }

    // Read and parse 18-byte TGA header (packed to avoid padding):contentReference[oaicite:19]{index=19}
#pragma pack(push,1)
    struct TGAHeader {
        uint8_t  identsize;
        uint8_t  colourmaptype;
        uint8_t  imagetype;
        uint16_t colourmapstart;
        uint16_t colourmaplength;
        uint8_t  colourmapbits;
        uint16_t xstart;
        uint16_t ystart;
        uint16_t width;
        uint16_t height;
        uint8_t  bits;
        uint8_t  descriptor;
    } header;
#pragma pack(pop)
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!file) {
        throw std::runtime_error("Failed to read TGA header");
    }

    // Skip image ID field if present
    if (header.identsize > 0) {
        file.seekg(header.identsize, std::ios::cur);
    }

    // Read palette (color map) if present
    std::vector<uint8_t> palette;
    if (header.colourmaptype == 1) {
        int palEntrySize = header.colourmapbits / 8; // bytes per palette entry
        int palCount = header.colourmaplength;
        int palStart = header.colourmapstart;
        palette.resize(palCount * palEntrySize);
        // Skip to first entry index
        if (palStart > 0) {
            file.seekg(palStart * palEntrySize, std::ios::cur);
        }
        file.read(reinterpret_cast<char*>(palette.data()), palette.size());
    }

    int width = header.width;
    int height = header.height;
    int pixelCount = width * height;
    bool isRLE = (header.imagetype & 0x08) != 0;      // types 9,10,11
    bool isColorMap = (header.imagetype == 1 || header.imagetype == 9);
    bool isTrueColor = (header.imagetype == 2 || header.imagetype == 10);
    bool isGray = (header.imagetype == 3 || header.imagetype == 11);

    // Determine how many bytes per pixel in the file (before any conversion)
    int fileBpp = (header.bits + 7) / 8;

    // Prepare a raw data buffer; if grayscale we use one byte per pixel, else 4 bytes per pixel (RGBA)
    std::vector<uint8_t> rawData;
    if (isGray) {
        rawData.resize(pixelCount); // one byte per pixel
    }
    else {
        rawData.resize(pixelCount * 4); // RGBA
    }

    // Helper to read a block of bytes
    auto readBytes = [&](void* dest, size_t size) {
        file.read(reinterpret_cast<char*>(dest), size);
        if (!file) throw std::runtime_error("Unexpected end of TGA pixel data");
        };

    // Decode pixel data (RLE or raw)
    if (isRLE) {
        // Run-length encoded decoding
        int pixelsDecoded = 0;
        while (pixelsDecoded < pixelCount) {
            uint8_t packetHeader = 0;
            file.read(reinterpret_cast<char*>(&packetHeader), 1);
            if (!file) throw std::runtime_error("Unexpected EOF in TGA RLE data");
            int count = (packetHeader & 0x7F) + 1;
            bool runPacket = (packetHeader & 0x80) != 0;
            if (runPacket) {
                // Read one pixel and repeat it 'count' times
                std::vector<uint8_t> pixel(fileBpp);
                readBytes(pixel.data(), fileBpp);
                for (int i = 0; i < count; i++) {
                    if (isColorMap) {
                        rawData[pixelsDecoded++] = pixel[0];
                    }
                    else if (isGray) {
                        rawData[pixelsDecoded++] = pixel[0];
                    }
                    else { // true-color
                        if (fileBpp == 3 || fileBpp == 4) {
                            uint8_t b = pixel[0];
                            uint8_t g = pixel[1];
                            uint8_t r = pixel[2];
                            uint8_t a = (fileBpp == 4) ? pixel[3] : 255;
                            int base = pixelsDecoded * 4;
                            rawData[base + 0] = r;
                            rawData[base + 1] = g;
                            rawData[base + 2] = b;
                            rawData[base + 3] = a;
                            pixelsDecoded++;
                        }
                        else if (fileBpp == 2) {
                            uint16_t v = *reinterpret_cast<uint16_t*>(pixel.data());
                            bool alphaBit = (v & 0x8000) != 0;
                            uint8_t b = (v & 0x1F) << 3;
                            uint8_t g = ((v >> 5) & 0x1F) << 3;
                            uint8_t r = ((v >> 10) & 0x1F) << 3;
                            uint8_t a = alphaBit ? 255 : 255;
                            int base = pixelsDecoded * 4;
                            rawData[base + 0] = r;
                            rawData[base + 1] = g;
                            rawData[base + 2] = b;
                            rawData[base + 3] = a;
                            pixelsDecoded++;
                        }
                    }
                }
            }
            else {
                // Raw packet: read 'count' separate pixels
                for (int i = 0; i < count; i++) {
                    if (isColorMap) {
                        uint8_t index = 0;
                        readBytes(&index, 1);
                        rawData[pixelsDecoded++] = index;
                    }
                    else if (isGray) {
                        uint8_t gray = 0;
                        readBytes(&gray, 1);
                        rawData[pixelsDecoded++] = gray;
                    }
                    else { // true-color
                        if (fileBpp == 3) {
                            uint8_t b, g, r;
                            readBytes(&b, 1);
                            readBytes(&g, 1);
                            readBytes(&r, 1);
                            int base = pixelsDecoded * 4;
                            rawData[base + 0] = r;
                            rawData[base + 1] = g;
                            rawData[base + 2] = b;
                            rawData[base + 3] = 255;
                            pixelsDecoded++;
                        }
                        else if (fileBpp == 4) {
                            uint8_t b, g, r, a;
                            readBytes(&b, 1);
                            readBytes(&g, 1);
                            readBytes(&r, 1);
                            readBytes(&a, 1);
                            int base = pixelsDecoded * 4;
                            rawData[base + 0] = r;
                            rawData[base + 1] = g;
                            rawData[base + 2] = b;
                            rawData[base + 3] = a;
                            pixelsDecoded++;
                        }
                        else if (fileBpp == 2) {
                            uint16_t v = 0;
                            readBytes(&v, 2);
                            bool alphaBit = (v & 0x8000) != 0;
                            uint8_t b = (v & 0x1F) << 3;
                            uint8_t g = ((v >> 5) & 0x1F) << 3;
                            uint8_t r = ((v >> 10) & 0x1F) << 3;
                            int base = pixelsDecoded * 4;
                            rawData[base + 0] = r;
                            rawData[base + 1] = g;
                            rawData[base + 2] = b;
                            rawData[base + 3] = 255;
                            pixelsDecoded++;
                        }
                    }
                }
            }
        }
    }
    else {
        // Uncompressed (raw) pixel data
        if (isColorMap) {
            // Read indices directly
            file.read(reinterpret_cast<char*>(rawData.data()), pixelCount);
            if (!file) throw std::runtime_error("Failed to read indexed pixels");
        }
        else if (isGray) {
            file.read(reinterpret_cast<char*>(rawData.data()), pixelCount);
            if (!file) throw std::runtime_error("Failed to read grayscale pixels");
        }
        else {
            // True-color raw
            if (header.bits == 24) {
                // 24-bit: BGR
                for (int i = 0; i < pixelCount; i++) {
                    uint8_t b, g, r;
                    readBytes(&b, 1);
                    readBytes(&g, 1);
                    readBytes(&r, 1);
                    int base = i * 4;
                    rawData[base + 0] = r;
                    rawData[base + 1] = g;
                    rawData[base + 2] = b;
                    rawData[base + 3] = 255;
                }
            }
            else if (header.bits == 32) {
                // 32-bit: BGRA
                file.read(reinterpret_cast<char*>(rawData.data()), pixelCount * 4);
                if (!file) throw std::runtime_error("Failed to read 32-bit pixels");
                // Swap B<->R channels
                for (int i = 0; i < pixelCount; i++) {
                    int base = i * 4;
                    std::swap(rawData[base + 0], rawData[base + 2]);
                }
            }
            else if (header.bits == 16 || header.bits == 15) {
                // 15/16-bit RGB
                for (int i = 0; i < pixelCount; i++) {
                    uint16_t v = 0;
                    readBytes(&v, 2);
                    bool alphaBit = (v & 0x8000) != 0;
                    uint8_t b = (v & 0x1F) << 3;
                    uint8_t g = ((v >> 5) & 0x1F) << 3;
                    uint8_t r = ((v >> 10) & 0x1F) << 3;
                    int base = i * 4;
                    rawData[base + 0] = r;
                    rawData[base + 1] = g;
                    rawData[base + 2] = b;
                    rawData[base + 3] = 255;
                }
            }
        }
    }

    // Convert indexed (palette) or use raw data directly
    std::vector<uint8_t> imageData;
    if (isColorMap) {
        // Convert indices to RGBA using the palette
        imageData.resize(pixelCount * 4);
        int palEntrySize = header.colourmapbits / 8;
        for (int i = 0; i < pixelCount; i++) {
            int idx = rawData[i];
            int p = idx * palEntrySize;
            uint8_t b = palette[p + 0];
            uint8_t g = palette[p + 1];
            uint8_t r = palette[p + 2];
            uint8_t a = (palEntrySize == 4) ? palette[p + 3] : 255;
            int base = i * 4;
            imageData[base + 0] = r;
            imageData[base + 1] = g;
            imageData[base + 2] = b;
            imageData[base + 3] = a;
        }
    }
    else if (isGray) {
        // For grayscale, keep one byte per pixel
        imageData = std::move(rawData);
    }
    else {
        // True-color data is already in RGBA format in rawData
        imageData = std::move(rawData);
    }

    // Vertical flip if origin is bottom-left (descriptor bit5 == 0):contentReference[oaicite:20]{index=20}
    bool flipVertical = ((header.descriptor & 0x20) == 0);
    if (flipVertical) {
        int rowBytes = isGray ? width : width * 4;
        std::vector<uint8_t> rowTemp(rowBytes);
        for (int y = 0; y < height / 2; y++) {
            uint8_t* top = imageData.data() + y * rowBytes;
            uint8_t* bot = imageData.data() + (height - 1 - y) * rowBytes;
            memcpy(rowTemp.data(), top, rowBytes);
            memcpy(top, bot, rowBytes);
            memcpy(bot, rowTemp.data(), rowBytes);
        }
    }

    // Create Direct3D texture
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    if (isGray) {
        desc.Format = DXGI_FORMAT_R8_UNORM;          // single channel
    }
    else {
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;  // RGBA 8-bit
    }
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // allow shader access:contentReference[oaicite:21]{index=21}
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = imageData.data();
    initData.SysMemPitch = isGray ? width : (width * 4);

    TextureResource resource;
	HRESULT hr = device->CreateTexture2D(&desc, &initData, &resource.Texture);
    if (FAILED(hr)) {
        throw std::runtime_error("ID3D11Device::CreateTexture2D failed");
    }

    // Create shader-resource-view (SRV):contentReference[oaicite:22]{index=22}
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    hr = device->CreateShaderResourceView(resource.Texture.Get(), &srvDesc, &resource.ShaderResourceView);
    if (FAILED(hr)) 
    {
        throw std::runtime_error("CreateShaderResourceView failed");
    }

    m_Cache[path] = std::move(resource);
    LOG_SUCCESS("LOADED TGA FILE!");
	return true;
}
