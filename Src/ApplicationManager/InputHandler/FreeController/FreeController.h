#pragma once
#include "ApplicationManager/InputHandler/InputHandler.h"
#include "RenderManager/Camera/CameraController.h"


class FreeController final: public IInputContext
{
public:
	FreeController() = default;
	void HandleInput(float deltaTime) override;

	void AttachCameraController(CameraController* cameraController);

	// Setters
	void SetMoveForwardKey(KeyCode key) { m_MoveForwardKey = key; }
	void SetMoveBackwardKey(KeyCode key) { m_MoveBackwardKey = key; }
	void SetMoveLeftKey(KeyCode key) { m_MoveLeftKey = key; }
	void SetMoveRightKey(KeyCode key) { m_MoveRightKey = key; }

	void SetMouseSensitivityX(float x) { m_MouseSensitivityX = x; }
	void SetMouseSensitivityY(float y) { m_MouseSensitivityY = y; }
	void SetMouseOnScreen(bool val);
	bool IsMouseOnScreen() const { return m_ThirdPersonView; }

	// Getters
	KeyCode GetMoveForwardKey()     const { return m_MoveForwardKey; }
	KeyCode GetMoveBackwardKey()    const { return m_MoveBackwardKey; }
	KeyCode GetMoveLeftKey()        const { return m_MoveLeftKey; }
	KeyCode GetMoveRightKey()       const { return m_MoveRightKey; }

	float GetMouseSensitivityX() const { return m_MouseSensitivityX; }
	float GetMouseSensitivityY() const { return m_MouseSensitivityY; }

private:
	void HandleMouseLook(float deltaTime) const;

private:
	KeyCode m_MoveForwardKey{ 'W' };
	KeyCode m_MoveBackwardKey{ 'S' };
	KeyCode m_MoveLeftKey{ 'A' };
	KeyCode m_MoveRightKey{ 'D' };
	float m_MouseSensitivityX = 0.8f;
	float m_MouseSensitivityY = 0.8f;
	bool m_ThirdPersonView{ false };

	CameraController* m_CameraController{ nullptr };
};
