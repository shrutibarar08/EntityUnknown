#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

#include "LevelEditorManager/Core/Commands/ICommand.h"

class BoundedUndoPolicy
{
public:
    static constexpr const char* PolicyName = "BoundedUndo";
    std::string name() const noexcept { return PolicyName; }

    // ---- Core limit ----
    std::size_t maxDepth = 256;

    // extend CommandStack to coalesce, read these flags there.
    bool   coalesceEnabled = true;   // allow squashing similar successive commands
    double coalesceWindowSeconds = 0.125;  // time window for squashing (UI-driven)
    
    // User-provided predicate to decide whether two commands can be coalesced.
    // Return true to merge (e.g., two SetTransform drags on same entity/axis).
    using CoalescePredicate = bool(*)(const ICommand& prev, const ICommand& next);
    CoalescePredicate coalescePredicate = nullptr;

    // ---- macro recording (group multiple commands as one) ----
    bool macrosEnabled = true;

    // Begin/End/Cancel are NOOP-safe CommandStack can check IsMacroRecording()
    void BeginMacro() noexcept { ++m_macroDepth; }
    void EndMacro()   noexcept { if (m_macroDepth) --m_macroDepth; }
    void CancelMacro() noexcept { m_macroDepth = 0; }

    [[nodiscard]] bool IsMacroRecording() const noexcept { return m_macroDepth > 0; }
    void Reset() noexcept { m_macroDepth = 0; }

    // RAII helper for scoped macro recordings:
    class MacroScope 
    {
    public:
        explicit MacroScope(BoundedUndoPolicy& p) noexcept : policy(&p) { policy->BeginMacro(); }
        MacroScope(const MacroScope&) = delete;
        MacroScope& operator=(const MacroScope&) = delete;
        MacroScope(MacroScope&& other) noexcept : policy(other.policy) { other.policy = nullptr; }
        MacroScope& operator=(MacroScope&& other) noexcept {
            if (this != &other) { EndIfNeeded(); policy = other.policy; other.policy = nullptr; }
            return *this;
        }
        ~MacroScope() { EndIfNeeded(); }
    private:
        void EndIfNeeded() noexcept { if (policy) { policy->EndMacro(); policy = nullptr; } }
        BoundedUndoPolicy* policy{};
    };

private:
    unsigned int m_macroDepth = 0; // supports nested BeginMacro/EndMacro
};
