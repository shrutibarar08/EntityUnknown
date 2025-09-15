#pragma once

#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include "Utils/HelperFunctions.h"

class LevelEditorContext;

// Base interface for editor tools
struct ITool
{
    virtual ~ITool() = default;
    virtual std::string_view Name() const = 0;
    // NOTE: pointer param to match editor usage: tool->Tick(m_pEditorContext.get())
    virtual void Tick(LevelEditorContext* ctx) = 0;
};

// Factory now returns ownership
using ToolFactory = std::function<std::unique_ptr<ITool>()>;

class ToolRegistry {
public:
    static ToolRegistry& Get() { static ToolRegistry r; return r; }

    // Registers or replaces a tool factory for 'name'
    void Register(std::string_view name, ToolFactory f) {
        const auto key = ct_hash(name.data());                 // if ct_hash supports (ptr,len), prefer that
        map[key] = Entry{ std::string(name), std::move(f) };
    }

    // Optional helper: list of tool names
    std::vector<std::string> Names() const {
        std::vector<std::string> n; n.reserve(map.size());
        for (const auto& kv : map) n.push_back(kv.second.name);
        return n;
    }

    // Creates a tool instance (caller takes ownership)
    std::unique_ptr<ITool> CreateLevel(std::string_view name) const {
        const auto it = map.find(ct_hash(name.data()));
        if (it == map.end() || !it->second.factory) return nullptr;
        return it->second.factory();
    }

private:
    struct Entry { std::string name; ToolFactory factory; };
    std::unordered_map<std::uint64_t, Entry> map;
};

// Simple static auto-registrar (works when Type is not namespaced)
#define REGISTER_TOOL(NameStr, Type)                                           \
    namespace {                                                                \
        struct AutoToolReg_##Type {                                            \
            AutoToolReg_##Type() {                                             \
                ToolRegistry::Get().Register(                                  \
                    NameStr, [](){ return std::make_unique<Type>(); }          \
                );                                                             \
            }                                                                  \
        };                                                                     \
        static AutoToolReg_##Type s_auto_tool_reg_##Type;                      \
    }
