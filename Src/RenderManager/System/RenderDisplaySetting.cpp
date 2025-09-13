#include "RenderDisplaySetting.h"
#include "RenderAdapter.h"
#include <windows.h>
#include "Utils/HelperFunctions.h"
#include "Utils/Logger/Logger.h"


int RenderMonitorSetting::Enumerate(const RenderAdapter& adapter)
{
    m_outputs.clear();
    m_selected = -1;
    m_refreshNum = 60;
    m_refreshDen = 1;

    auto sel = adapter.GetSelectedAdapter();
    if (!sel) return 0;

    for (UINT i = 0;; ++i)
    {
        Microsoft::WRL::ComPtr<IDXGIOutput> out;
        const HRESULT hr = sel->EnumOutputs(i, out.ReleaseAndGetAddressOf());
        if (hr == DXGI_ERROR_NOT_FOUND) break;
        if (FAILED(hr) || !out) continue;

        DXGI_OUTPUT_DESC d{};
        if (FAILED(out->GetDesc(&d))) continue;

        EU_RENDER_MONITOR_OUTPUT_DESC e{};
        e.output = out;
        e.desc = d;
        m_outputs.emplace_back(std::move(e));
    }
    return static_cast<int>(m_outputs.size());
}

Microsoft::WRL::ComPtr<IDXGIOutput> RenderMonitorSetting::GetSelectedOutput() const
{
    if (!HasSelection()) return {};
    return m_outputs[m_selected].output;
}

DXGI_OUTPUT_DESC RenderMonitorSetting::GetSelectedOutputDesc() const
{
    DXGI_OUTPUT_DESC d{};
    if (HasSelection()) d = m_outputs[m_selected].desc;
    return d;
}

bool RenderMonitorSetting::FindClosestMode(ID3D11Device* device,
    const DXGI_MODE_DESC& requested,
    DXGI_MODE_DESC& outMode)
{
    if (!HasSelection()) return false;

    DXGI_MODE_DESC req = requested;
    req.Width = 0; req.Height = 0;
    req.RefreshRate.Numerator = 0;
    req.RefreshRate.Denominator = 0;
    req.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    req.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    HRESULT hr = m_outputs[m_selected].output->FindClosestMatchingMode(&req, &outMode, device);
    if (FAILED(hr))
    {
        hr = m_outputs[m_selected].output->FindClosestMatchingMode(&req, &outMode, nullptr);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed To Find Closed Matching Model");
            return false;
        }
    }

    m_refreshNum = outMode.RefreshRate.Numerator;
    m_refreshDen = outMode.RefreshRate.Denominator;
    return true;
}

UINT RenderMonitorSetting::RefreshRateNumerator() const { return m_refreshNum; }
UINT RenderMonitorSetting::RefreshRateDenominator() const { return m_refreshDen; }

UINT RenderMonitorSetting::RefreshRateHzRounded() const
{
    if (m_refreshDen == 0) return 0;
    return static_cast<UINT>(static_cast<double>(m_refreshNum) / static_cast<double>(m_refreshDen) + 0.5);
}
