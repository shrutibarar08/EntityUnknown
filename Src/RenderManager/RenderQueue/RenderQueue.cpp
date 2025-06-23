#include "RenderQueue.h"

#include "ExceptionManager/IException.h"
#include "Utils/Logger/Logger.h"

#include <algorithm>
#include <commctrl.h>
#include <format>
#include <ranges>

void RenderQueueSingleton::Init(CameraController* controller, ID3D11Device* device, ID3D11DeviceContext* deviceContext,
	PhysicsSystem* physics)
{
    if (!m_Instance)
    {
        m_Instance.reset(new RenderQueueSingleton(controller, device, deviceContext, physics));
    }
#ifdef _DEBUG
    else
    {
        LOG_WARNING("Init called more than once for render queue\n");
    }
#endif
}

RenderQueueSingleton* RenderQueueSingleton::Get()
{
    if (!m_Instance) THROW("[RenderQueueSingleton] Error: Init() must be called before Get().");
    return m_Instance.get();
}

void RenderQueueSingleton::Shutdown()
{
    if (m_Instance)
    {
        m_Instance.reset();
    }
#ifdef _DEBUG
    else
    {
        LOG_WARNING("Shutdown called, but RenderQueueSingleton was never initialized\n");
    }
#endif
}

bool RenderQueueSingleton::IsInitialized()
{
    return m_Instance != nullptr;
}

bool RenderQueueSingleton::AddRender(IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    if (m_Renders.contains(id)) return false;
    m_Renders[id] = render;
    m_PhysicsSystem->AddObject(render);

    if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);

    return true;
}

bool RenderQueueSingleton::RemoveRender(const IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    if (!m_Renders.contains(id)) return false;
    return RemoveRender(id);
}

bool RenderQueueSingleton::RemoveRender(ID renderID)
{
    if (!m_Renders.contains(renderID)) return false;
    m_Renders.erase(renderID);
    m_PhysicsSystem->RemoveObject(renderID);
    return true;
}

bool RenderQueueSingleton::AddRenderBackground(IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    if (m_BackgroundRenders.contains(id)) return false;
    m_BackgroundRenders[id] = render;
    m_PhysicsSystem->AddObject(render);

    if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);

    return true;
}

bool RenderQueueSingleton::RemoveRenderBackground(const IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    return RemoveRenderBackground(id);
}

bool RenderQueueSingleton::RemoveRenderBackground(ID renderID)
{
    if (!m_BackgroundRenders.contains(renderID)) return false;
    m_BackgroundRenders.erase(renderID);
    m_PhysicsSystem->RemoveObject(renderID);
    return true;
}

bool RenderQueueSingleton::AddRenderFront(IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    if (m_FrontRenders.contains(id)) return false;
    m_FrontRenders[id] = render;
    m_PhysicsSystem->AddObject(render);

	if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);

    return true;
}

bool RenderQueueSingleton::RemoveRenderFront(const IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    return RemoveRenderFront(id);
}

bool RenderQueueSingleton::RemoveRenderFront(ID renderID)
{
    if (!m_FrontRenders.contains(renderID)) return false;
    m_FrontRenders.erase(renderID);
    m_PhysicsSystem->RemoveObject(renderID);
    return true;
}

bool RenderQueueSingleton::Update(UINT width, UINT height)
{
    m_ScreenHeight = height;
    m_ScreenWidth = width;

    //~ Update Objects on Space
    CAMERA_INFORMATION_CPU_DESC cb{};
    cb.ViewMatrix = XMMatrixTranspose(m_CameraController->GetViewMatrix());
    cb.ProjectionMatrix = XMMatrixTranspose(m_CameraController->GetProjectionMatrix());
    cb.CameraPosition = m_CameraController->GetEyePosition();

    m_Frustum.ConstructFromMatrix(
        m_CameraController->GetViewMatrix(),
        m_CameraController->GetProjectionMatrix(),
        m_CameraController->GetMaxVisibleDistance()
    );

    UpdateRenders(cb, m_Renders);

    //~ Update Objects on front or back
    cb.ProjectionMatrix = XMMatrixTranspose(m_CameraController->GetOrthogonalMatrix());
    UpdateRenders(cb, m_BackgroundRenders);
    UpdateRenders(cb, m_FrontRenders);

    return true;
}

bool RenderQueueSingleton::RenderBackground()
{
    //~ Get in Painters order
    std::vector<ID> painterOrder;
    ApplyPaintersAlgorithm(
        m_CameraController,
        m_BackgroundRenders,
        painterOrder,
        false);

    for (auto& renderID: painterOrder)
    {
        IRender* render = m_BackgroundRenders[renderID];
        if (!render->IsInitialized()) continue;
        render->Render(m_DeviceContext);
    }

    return true;
}

bool RenderQueueSingleton::Render()
{
	int counts = 0;
    //~ Render Solid Objects
    for (auto& render: m_Renders | std::views::values)
    {
        if (!render->IsInitialized() || render->IsTransparent()) continue;
        if (!IsInside(render)) continue;

        render->Render(m_DeviceContext);
        counts++;
    }

    //~ Get in Painters order
    std::vector<ID> painterOrder;
    ApplyPaintersAlgorithm(
        m_CameraController,
        m_Renders,
        painterOrder,
        true);

    for (auto& renderID : painterOrder)
    {
        IRender* render = m_Renders[renderID];
        if (!render->IsInitialized()) continue;
        render->Render(m_DeviceContext);
        counts++;
    }

    return true;
}

bool RenderQueueSingleton::RenderFront()
{
    //~ Get in Painters order
    std::vector<ID> painterOrder;
    ApplyPaintersAlgorithm(
        m_CameraController,
        m_FrontRenders,
        painterOrder,
        false);

    for (auto& renderID : painterOrder)
    {
        IRender* render = m_FrontRenders[renderID];
        if (!render || !render->IsInitialized()) continue;
        render->Render(m_DeviceContext);
    }

    return true;
}

bool RenderQueueSingleton::RenderShadowCast()
{
    if (m_LightSources.empty()) return false;

    for (auto& light: m_LightSources | std::views::values)
    {
        if (!light || !light->IsInitialized()) continue;
        if (!light->IsShadowDSVAssigned()) continue;

        light->UpdateProjectionMatrix(m_Frustum);

        SetRenderTargetToShadowMap(light->GetShadowDSV());
        ClearDepthStencilView(light->GetShadowDSV());

        const auto [width, height] = light->GetShadowResolution();

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<FLOAT>(width);
        viewport.Height = static_cast<FLOAT>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        m_DeviceContext->RSSetViewports(1, &viewport);

        // 4. Render shadow casters from light's view
        for (auto& render : m_Renders | std::views::values)
        {
            if (!render || !render->IsInitialized() || render->IsTransparent()) continue;
            if (!IsInside(render)) continue;
            render->RenderDepthOnly(m_DeviceContext, light->GetViewMatrix(), light->GetProjectionMatrix());
        }
    }
    return true;
}

bool RenderQueueSingleton::UnBind()
{
    for (auto& render: m_Renders | std::views::values)
    {
        render->UnBind(m_DeviceContext);
    }
    for (auto& render : m_BackgroundRenders | std::views::values)
    {
        render->UnBind(m_DeviceContext);
    }
    for (auto& render : m_FrontRenders | std::views::values)
    {
        render->UnBind(m_DeviceContext);
    }

    return true;
}

bool RenderQueueSingleton::CleanAll()
{
    CleanBackground();
    CleanFront();
    CleanSpace();
    return true;
}

bool RenderQueueSingleton::CleanBackground()
{
    m_BackgroundRenders.clear();
    return true;
}

bool RenderQueueSingleton::CleanSpace()
{
    m_Renders.clear();
    return true;
}

bool RenderQueueSingleton::CleanFront()
{
    m_FrontRenders.clear();
    return true;
}

bool RenderQueueSingleton::AddLight(ILightSource* light)
{
    ID lightID = light->GetAssignedID();
    if (m_LightSources.contains(lightID)) return false;
    m_LightSources[lightID] = light;

    return true;
}

bool RenderQueueSingleton::RemoveLight(const ILightSource* light)
{
    ID lightID = light->GetAssignedID();
    if (!m_LightSources.contains(lightID)) return false;
    return RemoveLight(lightID);
}

bool RenderQueueSingleton::RemoveLight(ID lightID)
{
    if (!m_LightSources.contains(lightID)) return false;
    m_LightSources.erase(lightID);
    return true;
}

bool RenderQueueSingleton::UpdateLight()
{
    for (auto& light : m_LightSources | std::views::values)
    {
        light->UpdateProjectionMatrix(m_Frustum);
    }
    return true;
}

RenderQueueSingleton::RenderQueueSingleton(
    CameraController* controller,
    ID3D11Device* device,
	ID3D11DeviceContext* deviceContext,
    PhysicsSystem* physics)
{
    m_CameraController = controller;
    m_DeviceContext = deviceContext;
    m_Device = device;
    m_PhysicsSystem = physics;
}

void RenderQueueSingleton::ApplyPaintersAlgorithm(
    const CameraController* controller,
    const RENDER_MAP& toRenderObject,
    std::vector<ID>& sortedRenders,
    bool accountTransparentOnly) const
{
    sortedRenders.clear();
    if (!controller) return;

    const DirectX::XMMATRIX viewMatrix = controller->GetViewMatrix();

    std::vector<std::pair<IRender*, float>> renderDepths;
    renderDepths.reserve(toRenderObject.size());

    for (auto& render : toRenderObject | std::views::values)
    {
        if (!render || !render->IsInitialized()) continue;

        if (accountTransparentOnly)
        {
            if (!render->IsTransparent()) continue;
            if (!IsInside(render)) continue;
        }

        const auto center = render->GetRigidBody()->GetTranslation();
        DirectX::XMVECTOR pos = XMLoadFloat3(&center);
        DirectX::XMVECTOR viewPos = XMVector3Transform(pos, viewMatrix);

        float depth = DirectX::XMVectorGetZ(viewPos); // Depth in camera space (negative == behind)
        renderDepths.emplace_back(render, depth);
    }

    // Sort from farthest to nearest (Painter's Order)
    std::ranges::sort(renderDepths, [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    sortedRenders.reserve(renderDepths.size());
    for (const auto& render : renderDepths | std::views::keys)
    {
        sortedRenders.push_back(render->GetAssignedID());
    }
}

void RenderQueueSingleton::UpdateRenders(
    const CAMERA_INFORMATION_CPU_DESC& desc,
    const RENDER_MAP& map)
{
    for (auto& render: map | std::views::values)
    {
        if (!render->IsInitialized()) continue;
        render->SetWorldMatrixData(desc);
        render->SetScreenHeight(m_ScreenHeight);
        render->SetScreenWidth(m_ScreenWidth);

        for (auto& light: m_LightSources | std::views::values)
        {
            render->AddLight(light);
        }
    }
}

bool RenderQueueSingleton::IsInside(IRender* render) const
{
    if (!render || !render->IsInitialized())
        return false;

    const DirectX::XMFLOAT3 center = render->GetRigidBody()->GetTranslation();
    const DirectX::XMFLOAT3 scale = render->GetScale();

    const DirectX::XMFLOAT3 halfScale =
    {
        scale.x,
        scale.y,
        scale.z
    };

    const DirectX::XMFLOAT3 min =
    {
        center.x - halfScale.x,
        center.y - halfScale.y,
        center.z - halfScale.z
    };

    const DirectX::XMFLOAT3 max =
    {
        center.x + halfScale.x,
        center.y + halfScale.y,
        center.z + halfScale.z
    };

    return m_Frustum.IntersectsAABB(min, max);
}

void RenderQueueSingleton::SetRenderTargetToShadowMap(ID3D11DepthStencilView* dsv) const
{
    if (dsv == nullptr) THROW("DSV was null SetRenderTargetToShadowMap");
    if (m_DeviceContext == nullptr) THROW("Device Context was null! SetRenderTargetToShadowMap");
    // we are only writing to depth
    m_DeviceContext->OMSetRenderTargets(0, nullptr, dsv);

    // Set viewport for shadow map rendering
    D3D11_VIEWPORT viewport = {};
    D3D11_TEXTURE2D_DESC texDesc = {};
    ID3D11Resource* res = nullptr;
    dsv->GetResource(&res);
    if (res)
    {
	    static_cast<ID3D11Texture2D*>(res)->GetDesc(&texDesc);
        res->Release();
    }

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(texDesc.Width);
    viewport.Height = static_cast<FLOAT>(texDesc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_DeviceContext->RSSetViewports(1, &viewport);
}

void RenderQueueSingleton::ClearDepthStencilView(ID3D11DepthStencilView* dsv) const
{
    m_DeviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
