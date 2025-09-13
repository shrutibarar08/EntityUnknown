#pragma once
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <dxgi.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cwctype>

struct EU_RENDER_ADAPTER_ENTRY
{
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    DXGI_ADAPTER_DESC                     desc      {};
    bool                                  isSoftware{};
};

template<typename P>
concept AdapterPolicy = requires(P policy, const std::vector<EU_RENDER_ADAPTER_ENTRY>&list)
{
    { policy(list) } -> std::same_as<int>;
};

class RenderAdapter
{
public:


public:
    int Enumerate(); // Enumerate system adapters. Returns count found.

    template<AdapterPolicy Policy>
    bool Select(const Policy& policy)
    {
        if (m_list.empty())
            Enumerate();

        const int idx = policy(m_list);
        if (idx >= 0 && idx < static_cast<int>(m_list.size()))
        {
            m_selected = idx;
            return true;
        }
        m_selected = -1;
        return false;
    }

    bool HasSelection () const;
    int  SelectedIndex() const;

    Microsoft::WRL::ComPtr<IDXGIAdapter> GetSelectedAdapter() const;
    DXGI_ADAPTER_DESC GetSelectedDesc() const;

    const std::vector<EU_RENDER_ADAPTER_ENTRY>& List() const { return m_list; }

private:
    std::vector<EU_RENDER_ADAPTER_ENTRY> m_list;
    int m_selected{ -1 };
};

// -------------------- Policies --------------------

// 1) Highest VRAM
struct Policy_HighestVRAM
{
    int operator()(const std::vector<EU_RENDER_ADAPTER_ENTRY>& list) const
    {
        if (list.empty()) return -1;
        auto it = std::max_element(list.begin(), list.end(),
            [](auto& A, auto& B) { return A.desc.DedicatedVideoMemory < B.desc.DedicatedVideoMemory; });
        return static_cast<int>(it - list.begin());
    }
};

// 2) By fixed index
struct Policy_ByIndex
{
    int index{ 0 };
    int operator()(const std::vector<EU_RENDER_ADAPTER_ENTRY>& list) const
    {
        return (index >= 0 && index < static_cast<int>(list.size())) ? index : -1;
    }
};

// 3) Name contains (case-insensitive; e.g., L"NVIDIA", L"AMD", L"Intel")
struct Policy_NameContains
{
    std::wstring needle;

    static std::wstring ToLower(std::wstring s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](wchar_t c) { return static_cast<wchar_t>(::towlower(c)); });
        return s;
    }

    int operator()(const std::vector<EU_RENDER_ADAPTER_ENTRY>& list) const
    {
        if (list.empty() || needle.empty()) return -1;
        const auto n = ToLower(needle);
        for (size_t i = 0; i < list.size(); ++i)
        {
            auto name = ToLower(list[i].desc.Description);
            if (name.find(n) != std::wstring::npos)
                return static_cast<int>(i);
        }
        return -1;
    }
};

// 4) Prefer discrete (non-software) then VRAM (simple heuristic)
struct Policy_PreferDiscrete
{
    int operator()(const std::vector<EU_RENDER_ADAPTER_ENTRY>& list) const
    {
        if (list.empty()) return -1;

        int best = -1;
        unsigned long long bestScore = 0;

        for (int i = 0; i < static_cast<int>(list.size()); ++i)
        {
            const auto& e = list[i];
            // Hardware gets big bonus over software (WARP)
            unsigned long long score = e.isSoftware ? 0ull : (1ull << 60);
            score += static_cast<unsigned long long>(e.desc.DedicatedVideoMemory);
            if (score > bestScore) { bestScore = score; best = i; }
        }
        return best;
    }
};
