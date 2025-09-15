#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class LevelEditorContext;
class Level;

using EntityID = std::uint64_t;

class NoopRenderHookPolicy
{
public:
    // Identity (useful in logs or debug UIs)
    std::string name() const noexcept;

    // --- Compatibility hooks (used by your LevelEditor today) ---
    void OnLevelLoaded() noexcept;
    void OnLevelSaved()  noexcept;

    // --- Richer hooks (optional; call if/when you wire them) ---
    void OnLevelLoaded(const Level* level, LevelEditorContext* ctx) noexcept;
    void OnLevelSaved(const Level* level, LevelEditorContext* ctx) noexcept;

    void OnSelectionChanged(const std::vector<EntityID>& newSelection,
        const std::vector<EntityID>& prevSelection,
        LevelEditorContext* ctx) noexcept;

    void OnEntityCreated(EntityID id, LevelEditorContext* ctx) noexcept;
    void OnEntityDeleted(EntityID id, LevelEditorContext* ctx) noexcept;

    void OnToolActivated(std::string_view toolName, LevelEditorContext* ctx) noexcept;
    void OnToolDeactivated(std::string_view toolName, LevelEditorContext* ctx) noexcept;

    // Per-frame hooks (e.g., for transient overlays or debug passes)
    void OnBeginFrame(LevelEditorContext* ctx) noexcept;
    void OnEndFrame(LevelEditorContext* ctx) noexcept;

    void OnDrawGizmos(LevelEditorContext* ctx) noexcept;
    void OnDrawOverlay(LevelEditorContext* ctx) noexcept;
};
