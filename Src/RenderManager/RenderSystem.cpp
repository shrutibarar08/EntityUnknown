#include "RenderSystem.h"

RenderSystem::RenderSystem(WindowsSystem* winSystem)
	: m_WindowsSystem(winSystem)
{
}

bool RenderSystem::OnInit(const SweetLoader& sweetLoader)
{
	return true;
}

bool RenderSystem::OnTick(float deltaTime)
{
	return true;
}

bool RenderSystem::OnExit(SweetLoader& sweetLoader)
{
	return true;
}

std::string RenderSystem::GetSystemName()
{
	return "RenderSystem";
}

ID3D11Device* RenderSystem::GetDevice() const
{
}

ID3D11DeviceContext* RenderSystem::GetDeviceContext() const
{
}

void RenderSystem::AttachSystemToRender(IRender* sysToRender)
{
}

void RenderSystem::RemoveSystemToRender(IRender* sysToRender)
{
}

void RenderSystem::RemoveSystemToRender(ID id)
{
}

bool RenderSystem::QueryAndStoreAdapter()
{
}

bool RenderSystem::QueryAndStoreMonitorDisplay()
{
}

bool RenderSystem::InitDeviceAndContext()
{
}

bool RenderSystem::InitSwapChain()
{
}

bool RenderSystem::InitRenderTargetView()
{
}

bool RenderSystem::InitDepthAndStencilView()
{
}

bool RenderSystem::InitViewport()
{
}

bool RenderSystem::InitRasterizationState()
{
}

void RenderSystem::CleanBuffers()
{
}

void RenderSystem::SetOMStates()
{
}

void RenderSystem::BeginRender()
{
}

void RenderSystem::ExecuteRender()
{
}

void RenderSystem::EndRender()
{
}
