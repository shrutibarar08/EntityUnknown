#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

static std::string WideToUTF8(const std::wstring& w)
{
    if (w.empty()) return {};
    int size = ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string out(size, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), out.data(), size, nullptr, nullptr);
    return out;
}


template<class...Ts> struct type_list{};

constexpr uint64_t ct_hash(const char* str)
{
    uint64_t hash = 1469598103934665603ull;
    while (*str)
    {
        hash ^= static_cast<unsigned char>(*str++);
        hash *= 1099511628211ull;
    }
    return hash;
}
