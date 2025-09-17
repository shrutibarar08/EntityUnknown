#include "EditorInputHandler.h"

#include "RenderManager/RenderQueue/RenderQueue.h"
#include "Editor/Core/EditorContext.h"


void EditorInputHandler::HandleInput(float deltaTime)
{
    CameraController* cam = RenderQueue::Get()->GetCameraController();
    if (!cam || !m_KeyboardHandler || !m_MouseHandler) return;

    if (m_KeyboardHandler->WasKeyPressed(VK_SPACE))
    {
        m_bEnableInputs = !m_bEnableInputs;
    }
    if (!m_bEnableInputs) return;

    float speed = cam->GetMovementSpeed() * deltaTime;
    if (m_KeyboardHandler->IsKeyDown(VK_SHIFT))   speed *= 2.0f;
    if (m_KeyboardHandler->IsKeyDown(VK_CONTROL)) speed *= 0.5f;

    if (m_KeyboardHandler->IsKeyDown('W')) cam->MoveForward(+speed);
    if (m_KeyboardHandler->IsKeyDown('S')) cam->MoveForward(-speed);
    if (m_KeyboardHandler->IsKeyDown('D')) cam->MoveRight(+speed);
    if (m_KeyboardHandler->IsKeyDown('A')) cam->MoveRight(-speed);
    if (m_KeyboardHandler->IsKeyDown('E')) cam->MoveUp(+speed);
    if (m_KeyboardHandler->IsKeyDown('Q')) cam->MoveUp(-speed);

    static bool firstLock = true;
    m_MouseHandler->UnHideCursor();
    m_MouseHandler->LockCursorToWindow(firstLock);
    firstLock = false;

    int dx = 0, dy = 0;
    m_MouseHandler->GetRawDelta(dx, dy);

    constexpr float kRotPerCount = 0.10f;
    cam->RotateYaw(dx * kRotPerCount * deltaTime);
    cam->RotatePitch(dy * kRotPerCount * deltaTime);
}

void EditorInputHandler::HandleEditorInputs(LevelEditorContext* context)
{
	if (!context) return;

	auto* cs = context->GetCommandStack();
	if (!cs) return;

	if (m_KeyboardHandler->WasChordPressed('Z', KeyboardHandler::Ctrl))
	{
		if (cs->CanUndo()) cs->Undo(context);
	}
	if (m_KeyboardHandler->WasChordPressed('Y', KeyboardHandler::Ctrl))
	{
		if (cs->CanRedo()) cs->Redo(context);
	}
	if (m_KeyboardHandler->WasChordPressed('S', KeyboardHandler::Ctrl))
	{
		if (auto* store = context->GetStoragePolicy())
		{
			store->Save(context);
		}
	}
}
