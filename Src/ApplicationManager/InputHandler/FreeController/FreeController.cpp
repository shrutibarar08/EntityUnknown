#include "FreeController.h"
#include "Utils/Logger/Logger.h"


void FreeController::HandleInput(float deltaTime)
{
	if (!m_CameraController) return;

	if (m_KeyboardHandler->WasKeyPressed(VK_SPACE))
	{
		LOG_INFO("Spaced Pressed!");
		m_ThirdPersonView = !m_ThirdPersonView;
	}

	if (!m_ThirdPersonView) return;

	DirectX::XMVECTOR moveDir = DirectX::XMVectorZero();
	if (m_KeyboardHandler->IsKeyDown(m_MoveForwardKey))    m_CameraController->MoveForward(deltaTime);
	if (m_KeyboardHandler->IsKeyDown(m_MoveBackwardKey))   m_CameraController->MoveForward(-deltaTime);
	if (m_KeyboardHandler->IsKeyDown(m_MoveLeftKey))	   m_CameraController->MoveRight(-deltaTime);
	if (m_KeyboardHandler->IsKeyDown(m_MoveRightKey))	   m_CameraController->MoveRight(deltaTime);

	// if (m_ThirdPersonView) HandleMouseLook(deltaTime);
	m_KeyboardHandler->EndFrame();
}

void FreeController::AttachCameraController(CameraController* cameraController)
{
	m_CameraController = cameraController;
}

void FreeController::SetMouseOnScreen(bool val)
{
	if (val)
	{
		m_MouseHandler->EndFrame();
		m_ThirdPersonView = true;
		LOG_INFO("Turned On Mouse!");
	}
	else
	{
		m_ThirdPersonView = false;
	};
}

void FreeController::HandleMouseLook(float deltaTime) const
{
	int dx = 0, dy = 0;
	m_MouseHandler->GetRawDelta(dx, dy);

	if (dx == 0 && dy == 0) return;
	if (!m_CameraController) return;

	float smoothing = 0.5f; // between 0.0 and 1.0
	static float smoothedDx = 0, smoothedDy = 0;

	smoothedDx = smoothedDx * (1.0f - smoothing) + dx * smoothing;
	smoothedDy = smoothedDy * (1.0f - smoothing) + dy * smoothing;

	float yawDelta = smoothedDx * m_MouseSensitivityX * 0.001f;
	float pitchDelta = smoothedDy * m_MouseSensitivityY * 0.001f;

	m_CameraController->RotateYaw(yawDelta);
	m_CameraController->RotatePitch(pitchDelta);

	m_MouseHandler->EndFrame();
}
