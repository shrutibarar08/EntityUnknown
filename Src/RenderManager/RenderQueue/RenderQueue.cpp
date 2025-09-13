#include "RenderQueue.h"

#include "ExceptionManager/IException.h"
#include "Utils/Logger/Logger.h"

#include <algorithm>
#include <commctrl.h>
#include <format>
#include <ranges>

#include "RenderManager/System/EURenderTarget.h"

void RenderQueue::Init(
    CameraController* controller,
    ID3D11Device* device,
    ID3D11DeviceContext* deviceContext,
	PhysicsSystem* physics)
{
    if (!m_Instance)
    {
        m_Instance.reset(new RenderQueue(controller, device, deviceContext, physics));
    }
#ifdef _DEBUG
    else
    {
        LOG_WARNING("Init called more than once for render queue\n");
    }
#endif
}

RenderQueue* RenderQueue::Get()
{
    if (!m_Instance) THROW("[RenderQueueSingleton] Error: Init() must be called before Get().");
    return m_Instance.get();
}

void RenderQueue::Shutdown()
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

bool RenderQueue::IsInitialized()
{
    return m_Instance != nullptr;
}

CameraController* RenderQueue::GetCameraController() const
{
    return m_CameraController;
}

bool RenderQueue::AddRender(IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    if (m_Renders.contains(id)) return false;
    m_Renders[id] = render;
    m_PhysicsSystem->AddObject(render);

    if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);

    return true;
}

bool RenderQueue::RemoveRender(const IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    if (!m_Renders.contains(id)) return false;
    return RemoveRender(id);
}

bool RenderQueue::RemoveRender(ID renderID)
{
    if (!m_Renders.contains(renderID)) return false;
    m_Renders.erase(renderID);
    m_PhysicsSystem->RemoveObject(renderID);
    return true;
}

bool RenderQueue::AddRenderBackground(IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    if (m_BackgroundRenders.contains(id)) return false;
    m_BackgroundRenders[id] = render;
    m_PhysicsSystem->AddObject(render);

    if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);

    return true;
}

bool RenderQueue::RemoveRenderBackground(const IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    return RemoveRenderBackground(id);
}

bool RenderQueue::RemoveRenderBackground(ID renderID)
{
    if (!m_BackgroundRenders.contains(renderID)) return false;
    m_BackgroundRenders.erase(renderID);
    m_PhysicsSystem->RemoveObject(renderID);
    return true;
}

bool RenderQueue::AddRenderFront(IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    if (m_FrontRenders.contains(id)) return false;
    m_FrontRenders[id] = render;
    m_PhysicsSystem->AddObject(render);

	if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);

    return true;
}

bool RenderQueue::RemoveRenderFront(const IRender* render)
{
    if (!render) return false;

    ID id = render->GetAssignedID();
    return RemoveRenderFront(id);
}

bool RenderQueue::RemoveRenderFront(ID renderID)
{
    if (!m_FrontRenders.contains(renderID)) return false;
    m_FrontRenders.erase(renderID);
    m_PhysicsSystem->RemoveObject(renderID);
    return true;
}

bool RenderQueue::Update(UINT width, UINT height)
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

bool RenderQueue::RenderBackground()
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
        if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);
        if (!render->IsInitialized()) continue;
        render->Render(m_DeviceContext);
    }

    return true;
}

bool RenderQueue::Render()
{
	int counts = 0;
    //~ Render Solid Objects
    for (auto& render: m_Renders | std::views::values)
    {
        if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);
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

bool RenderQueue::RenderFront()
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
        if (!render->IsInitialized()) render->Build(m_Device, m_DeviceContext);
        if (!render || !render->IsInitialized()) continue;
        render->Render(m_DeviceContext);
    }

    return true;
}

bool RenderQueue::RenderShadowCast()
{
    return true;
}

bool RenderQueue::RenderPostEffects(EURenderTarget& src, EURenderTarget& dst)
{
    if (!m_Device || !m_DeviceContext) return false;

    std::vector<IPostEffect*> chain;
    chain.reserve(m_PostEffects.size());
    for (auto& kv : m_PostEffects)
    {
        IPostEffect* fx = kv.second;
        if (fx && fx->IsEnabled())
            chain.push_back(fx);
    }

    if (chain.empty())
    {
        return true;
    }

    std::sort(chain.begin(), chain.end(),
        [](const IPostEffect* a, const IPostEffect* b) { return a->GetAssignedID() < b->GetAssignedID(); });

    bool srcToDst = true;
    unsigned applied = 0;

    for (IPostEffect* fx : chain)
    {
        EURenderTarget& inRT = srcToDst ? src : dst;
        EURenderTarget& outRT = srcToDst ? dst : src;

        if (!fx->Apply(m_Device, m_DeviceContext, inRT, outRT))
        {
            return false;
        }

        srcToDst = !srcToDst;
        ++applied;
    }

    if ((applied & 1u) == 0u)
    {
        ID3D11Texture2D* srcTex = src.ColorTex();
        ID3D11Texture2D* dstTex = dst.ColorTex();
        if (!srcTex || !dstTex)
        {
            return false;
        }

        UnBind();
        ID3D11ShaderResourceView* nullSRV = nullptr;
        m_DeviceContext->PSSetShaderResources(0, 1, &nullSRV);
        m_DeviceContext->CopyResource(dstTex, srcTex);
    }

    return true;
}

bool RenderQueue::UnBind()
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

bool RenderQueue::CleanAll()
{
    CleanBackground();
    CleanFront();
    CleanSpace();
    CleanPostEffects();
    return true;
}

bool RenderQueue::CleanBackground()
{
    m_BackgroundRenders.clear();
    return true;
}

bool RenderQueue::CleanSpace()
{
    m_Renders.clear();
    return true;
}

bool RenderQueue::CleanFront()
{
    m_FrontRenders.clear();
    return true;
}

bool RenderQueue::CleanPostEffects()
{
    m_PostEffects.clear();
    return true;
}

bool RenderQueue::AddLight(ILightSource* light)
{
    ID lightID = light->GetAssignedID();
    if (m_LightSources.contains(lightID)) return false;
    m_LightSources[lightID] = light;

    return true;
}

bool RenderQueue::RemoveLight(const ILightSource* light)
{
    ID lightID = light->GetAssignedID();
    if (!m_LightSources.contains(lightID)) return false;
    return RemoveLight(lightID);
}

bool RenderQueue::RemoveLight(ID lightID)
{
    if (!m_LightSources.contains(lightID)) return false;
    m_LightSources.erase(lightID);
    return true;
}

bool RenderQueue::UpdateLight()
{
    for (auto& light : m_LightSources | std::views::values)
    {
        light->UpdateProjectionMatrix(m_Frustum);
    }
    return true;
}

bool RenderQueue::AddPostEffect(IPostEffect* fx)
{
    if (!fx) { LOG_ERROR("AddPostEffect: null"); return false; }

    const ID id = fx->GetAssignedID();
    auto [it, inserted] = m_PostEffects.emplace(id, fx);
    if (!inserted)
    {
        it->second = fx;
    }
    return true;
}


bool RenderQueue::RemovePostEffect(const IPostEffect* fx)
{
    if (!fx) return false;
    return RemovePostEffect(fx->GetAssignedID());
}

bool RenderQueue::RemovePostEffect(ID fxID)
{
    const auto it = m_PostEffects.find(fxID);
    if (it == m_PostEffects.end())
        return false;
    m_PostEffects.erase(it);
    //LOG_SUCCESS("RemovePostEffect: id={}", static_cast<int>(fxID));
    return true;
}

RenderQueue::RenderQueue(
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

void RenderQueue::ApplyPaintersAlgorithm(
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

void RenderQueue::UpdateRenders(
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

bool RenderQueue::IsInside(IRender* render) const
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

void RenderQueue::SetRenderTargetToShadowMap(ID3D11DepthStencilView* dsv) const
{
    
}

void RenderQueue::ClearDepthStencilView(ID3D11DepthStencilView* dsv) const
{
    m_DeviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
