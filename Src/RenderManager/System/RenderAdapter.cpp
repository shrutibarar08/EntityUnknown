#include "RenderAdapter.h"

int RenderAdapter::Enumerate()
{
    m_list.clear();
    m_selected = -1;

    Microsoft::WRL::ComPtr<IDXGIFactory1> f1;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(f1.ReleaseAndGetAddressOf()))))
        return 0;

    for (UINT i = 0;; ++i)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> a1;
        if (f1->EnumAdapters1(i, a1.ReleaseAndGetAddressOf()) == DXGI_ERROR_NOT_FOUND)
            break;

        // Get DESC1 (for Flags) and DESC (for your storage)
        DXGI_ADAPTER_DESC1 d1{};
        if (FAILED(a1->GetDesc1(&d1)))
            continue;

        DXGI_ADAPTER_DESC d{};
        if (FAILED(a1->GetDesc(&d)))
            continue;

        EU_RENDER_ADAPTER_ENTRY e{};
        e.adapter = a1;
        e.desc = d;                                   // store DESC
        e.isSoftware = (d1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0; // remember software flag
        m_list.emplace_back(std::move(e));
    }
    return static_cast<int>(m_list.size());
}

bool RenderAdapter::HasSelection() const
{
    return m_selected >= 0;
}

int RenderAdapter::SelectedIndex() const
{
    return m_selected;
}

Microsoft::WRL::ComPtr<IDXGIAdapter> RenderAdapter::GetSelectedAdapter() const
{
    if (!HasSelection()) return {};
    Microsoft::WRL::ComPtr<IDXGIAdapter> base;
    m_list[m_selected].adapter.As(&base);
    return base;
}

DXGI_ADAPTER_DESC RenderAdapter::GetSelectedDesc() const
{
    DXGI_ADAPTER_DESC d{};
    if (HasSelection()) d = m_list[m_selected].desc;
    return d;
}
