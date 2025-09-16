#pragma once

#include "SystemManager/Registry/RegistryTool.h"

struct SpriteTool : ITool
{
    std::string_view Name() const override { return "Sprite Tool"; }
    void Tick(LevelEditorContext* ctx) override
    {}
};

REGISTER_TOOL("Sprite Tool", SpriteTool)

struct CompileShaderTool : ITool {
    std::string_view Name() const override { return "Compile Shader"; }
    void Tick(LevelEditorContext* ctx) override {
        // draw UI + run compile tasks
        ImGui::Text("Shader compilation panel...");
    }
};
REGISTER_TOOL("Compile Shader", CompileShaderTool);

struct CreateMeshTool : ITool {
    std::string_view Name() const override { return "Create Mesh"; }
    void Tick(LevelEditorContext* ctx) override {
        ImGui::Text("Mesh creation panel...");
    }
};
REGISTER_TOOL("Create Mesh", CreateMeshTool);
