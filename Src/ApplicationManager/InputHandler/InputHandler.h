#pragma once
#include "SystemManager/ISystem.h"
#include "WindowsManager/WindowsSystem.h"

using KeyCode = int;

class IInputContext: public PrimaryID
{
public:
	virtual ~IInputContext() = default;
	virtual void HandleInput(float deltaTime) = 0;

	void AttachKeyboardHandler(KeyboardHandler* keyboard) { m_KeyboardHandler = keyboard; }
	void AttachMouseHandler(MouseHandler* mouse) { m_MouseHandler = mouse; }

protected:
	KeyboardHandler* m_KeyboardHandler{ nullptr };
	MouseHandler* m_MouseHandler{ nullptr };
};

class InputHandler final: public ISystem
{
public:
	InputHandler(WindowsSystem* windowsSystem);
	~InputHandler() override = default;

	InputHandler(const InputHandler&) = delete;
	InputHandler(InputHandler&&) = delete;
	InputHandler& operator=(const InputHandler&) = delete;
	InputHandler& operator=(InputHandler&&) = delete;

	bool OnInit(const SweetLoader& sweetLoader) override;
	bool OnTick(float deltaTime) override;
	bool OnExit(SweetLoader& sweetLoader) override;
	std::string GetSystemName() override;

	void FocusControlOn(ID focusId);
	auto AddInputController(IInputContext* inputController) -> void;

protected:
	WindowsSystem* m_WindowsSystem{ nullptr };
	std::unordered_map<ID, IInputContext*> m_InputControllers{};
	IInputContext* m_ActiveInputController{ nullptr };
};
