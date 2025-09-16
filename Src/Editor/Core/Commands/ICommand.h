#pragma once

class LevelEditorContext;

class __declspec(novtable) ICommand
{
public:
    virtual ~ICommand() = default;

    virtual const char* GetCommandName() const noexcept = 0;
    virtual void Do(LevelEditorContext* ctx)            = 0;
    virtual void Undo(LevelEditorContext* ctx)          = 0;
};
