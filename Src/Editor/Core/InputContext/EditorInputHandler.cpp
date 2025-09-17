#include "EditorInputHandler.h"

#include "RenderManager/RenderQueue/RenderQueue.h"
#include "Editor/Core/EditorContext.h"


void EditorInputHandler::HandleInput(float deltaTime)
{
    CameraController* cam = RenderQueue::Get()->GetCameraController();
    if (!cam || !m_KeyboardHandler) return;

    float baseSpeed = cam->GetMovementSpeed();
    float speed = baseSpeed * deltaTime;

    // Speed modifiers
    if (m_KeyboardHandler->IsKeyDown(VK_SHIFT))
        speed *= 2.0f; // sprint
    if (m_KeyboardHandler->IsKeyDown(VK_CONTROL))
        speed *= 0.5f; // slow walk

    // Forward / backward
    if (m_KeyboardHandler->IsKeyDown('W'))
        cam->MoveForward(speed);
    if (m_KeyboardHandler->IsKeyDown('S'))
        cam->MoveForward(-speed);

    // Right / left
    if (m_KeyboardHandler->IsKeyDown('D'))
        cam->MoveRight(speed);
    if (m_KeyboardHandler->IsKeyDown('A'))
        cam->MoveRight(-speed);

    // Up / down
    if (m_KeyboardHandler->IsKeyDown('E'))
        cam->MoveUp(speed);
    if (m_KeyboardHandler->IsKeyDown('Q'))
        cam->MoveUp(-speed);
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
