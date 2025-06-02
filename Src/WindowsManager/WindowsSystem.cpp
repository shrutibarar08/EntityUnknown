#include "WindowsSystem.h"

bool WindowsSystem::OnInit(const SweetLoader& sweetLoader)
{
	return true;
}

bool WindowsSystem::OnTick(float deltaTime)
{
	return true;
}

bool WindowsSystem::OnExit(SweetLoader& sweetLoader)
{
	return true;
}

HWND WindowsSystem::GetWindowHandle() const
{
	return m_WindowHandle;
}

HINSTANCE WindowsSystem::GetWindowInstance() const
{
	return m_WindowInstance;
}
