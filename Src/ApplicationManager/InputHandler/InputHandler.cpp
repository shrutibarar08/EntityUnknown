#include "InputHandler.h"

InputHandler::InputHandler(WindowsSystem* windowsSystem)
	: m_WindowsSystem(windowsSystem)
{
}

bool InputHandler::OnInit(const SweetLoader& sweetLoader)
{
	return true;
}

bool InputHandler::OnTick(float deltaTime)
{
	if (m_ActiveInputController)
	{
		m_ActiveInputController->HandleInput(deltaTime);
	}
	return true;
}

bool InputHandler::OnExit(SweetLoader& sweetLoader)
{
	return true;
}

std::string InputHandler::GetSystemName()
{
	return "Input Handler";
}

void InputHandler::FocusControlOn(ID focusId)
{
	if (m_InputControllers.contains(focusId))
	{
		m_ActiveInputController = m_InputControllers[focusId];
	}
}

void InputHandler::AddInputController(IInputContext* inputController)
{
	if (!m_InputControllers.contains(inputController->GetAssignedID()))
	{
		m_InputControllers[inputController->GetAssignedID()] = inputController;

		inputController->AttachKeyboardHandler(&m_WindowsSystem->Keyboard);
		inputController->AttachMouseHandler(&m_WindowsSystem->Mouse);

		if (!m_ActiveInputController) m_ActiveInputController  = inputController;
	}
}
