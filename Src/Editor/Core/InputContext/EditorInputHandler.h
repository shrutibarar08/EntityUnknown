#pragma once

#include "ApplicationManager/InputHandler/InputHandler.h"

class LevelEditorContext;

class EditorInputHandler final: public IInputContext
{
public:
	void HandleInput(float deltaTime) override;
	void HandleEditorInputs(LevelEditorContext* context);

private:
	bool m_bEnableInputs{ false };
};
