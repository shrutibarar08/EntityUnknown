#pragma once

#include "SystemManager/Registry/RegistryTool.h"

struct SpriteTool : ITool
{
    std::string_view Name() const override { return "Sprite Tool"; }
    void Tick(LevelEditorContext* ctx) override
    {}
};

REGISTER_TOOL("Sprite Tool", SpriteTool)
