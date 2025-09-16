#include "NoopRenderHookPolicy.h"

std::string NoopRenderHookPolicy::name() const noexcept 
{
    return "NoopRenderHooks";
}

// --- Compatibility hooks (currently used by LevelEditor) ---
void NoopRenderHookPolicy::OnLevelLoaded() noexcept {}
void NoopRenderHookPolicy::OnLevelSaved() noexcept {}

void NoopRenderHookPolicy::OnLevelLoaded(const Level*, LevelEditorContext*) noexcept {}
void NoopRenderHookPolicy::OnLevelSaved(const Level*, LevelEditorContext*) noexcept {}

void NoopRenderHookPolicy::OnSelectionChanged(const std::vector<EntityID>&,
    const std::vector<EntityID>&,
    LevelEditorContext*) noexcept
{}

void NoopRenderHookPolicy::OnEntityCreated(EntityID, LevelEditorContext*) noexcept {}
void NoopRenderHookPolicy::OnEntityDeleted(EntityID, LevelEditorContext*) noexcept {}

void NoopRenderHookPolicy::OnToolActivated(std::string_view, LevelEditorContext*)   noexcept {}
void NoopRenderHookPolicy::OnToolDeactivated(std::string_view, LevelEditorContext*) noexcept {}

void NoopRenderHookPolicy::OnBeginFrame(LevelEditorContext*) noexcept {}
void NoopRenderHookPolicy::OnEndFrame(LevelEditorContext*)   noexcept {}

void NoopRenderHookPolicy::OnDrawGizmos(LevelEditorContext*)  noexcept {}
void NoopRenderHookPolicy::OnDrawOverlay(LevelEditorContext*) noexcept {}
