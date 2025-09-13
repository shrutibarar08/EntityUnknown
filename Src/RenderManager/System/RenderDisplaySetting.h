#pragma once
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cwctype>


class RenderAdapter;

struct EU_RENDER_MONITOR_OUTPUT_DESC
{
    Microsoft::WRL::ComPtr<IDXGIOutput> output;
    DXGI_OUTPUT_DESC                    desc{};
};

template<typename P>
concept OutputPolicy = requires(P policy, const std::vector<EU_RENDER_MONITOR_OUTPUT_DESC>&list)
{
    { policy(list) } -> std::same_as<int>;
};

class RenderMonitorSetting
{
public:
    // Enumerate outputs from the currently selected adapter in RenderAdapter.
    int  Enumerate(const RenderAdapter& adapter);

    // Policy-based selection
    template<OutputPolicy Policy>
    bool Select(const Policy& policy)
    {
        if (m_outputs.empty()) return false;

        const int idx = policy(m_outputs);
        if (idx >= 0 && idx < static_cast<int>(m_outputs.size()))
        {
            m_selected = idx;
            return true;
        }
        m_selected = -1;
        return false;
    }

    bool HasSelection () const { return m_selected >= 0; }
    int  SelectedIndex() const { return m_selected;      }

    Microsoft::WRL::ComPtr<IDXGIOutput> GetSelectedOutput    () const;
    DXGI_OUTPUT_DESC                    GetSelectedOutputDesc() const;

    bool FindClosestMode(ID3D11Device* device,
        const DXGI_MODE_DESC& requested,
        DXGI_MODE_DESC& outMode);

    UINT RefreshRateNumerator  () const;
    UINT RefreshRateDenominator() const;
    UINT RefreshRateHzRounded  () const;

private:
    std::vector<EU_RENDER_MONITOR_OUTPUT_DESC> m_outputs;
    int  m_selected  { -1 };
    UINT m_refreshNum{ 60 };
    UINT m_refreshDen{ 1 };
};

// -------------------- Tiny Output Policies --------------------

// 1) Primary (often the first enumerated)
struct OutputPolicy_Primary
{
    int operator()(const std::vector<EU_RENDER_MONITOR_OUTPUT_DESC>& list) const
    {
        return list.empty() ? -1 : 0;
    }
};

// 2) By fixed index
struct OutputPolicy_ByIndex
{
    int index{ 0 };
    int operator()(const std::vector<EU_RENDER_MONITOR_OUTPUT_DESC>& list) const
    {
        return (index >= 0 && index < static_cast<int>(list.size())) ? index : -1;
    }
};

// 3) Name contains (case-insensitive) — matches DXGI_OUTPUT_DESC::DeviceName (e.g., "\\.\DISPLAY1")
struct OutputPolicy_NameContains
{
    std::wstring needle;

    static std::wstring ToLower(std::wstring s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](wchar_t c) { return static_cast<wchar_t>(::towlower(c)); });
        return s;
    }

    int operator()(const std::vector<EU_RENDER_MONITOR_OUTPUT_DESC>& list) const
    {
        if (list.empty() || needle.empty()) return -1;
        const auto n = ToLower(needle);
        for (size_t i = 0; i < list.size(); ++i)
        {
            auto name = ToLower(std::wstring(list[i].desc.DeviceName));
            if (name.find(n) != std::wstring::npos)
                return static_cast<int>(i);
        }
        return -1;
    }
};
