#pragma once
#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include "Utils/HelperFunctions.h"


struct ITool
{
    virtual ~ITool() = default;
    virtual std::string_view Name() const = 0;
};

using ToolFactory = std::function<std::unique_ptr<ITool>()>;

class ToolRegistry
{
public:
    static ToolRegistry& Get()
    {
        static ToolRegistry r;
        return r;
    }

    bool Register(std::string_view name, ToolFactory f)
    {
        const auto key = ct_hash(name.data());
        auto [it, inserted] = map.emplace(key, Entry{ std::string(name), std::move(f) });
        if (!inserted)
        {
            it->second.name = std::string(name);
            it->second.factory = std::move(f);
        }
        return inserted;
    }

    std::vector<std::string> Names() const
    {
        std::vector<std::string> n; n.reserve(map.size());
        for (const auto& kv : map) n.push_back(kv.second.name);
        return n;
    }

    std::unique_ptr<ITool> CreateTool(std::string_view name) const
    {
        const auto it = map.find(ct_hash(name.data()));
        if (it == map.end() || !it->second.factory) return nullptr;
        return it->second.factory();
    }

private:
    struct Entry { std::string name; ToolFactory factory; };
    std::unordered_map<std::uint64_t, Entry> map;
};

#define REGISTER_TOOL(NameStr, Type)                                           \
    namespace {                                                                \
        struct AutoToolReg_##Type {                                            \
            AutoToolReg_##Type() {                                             \
                (void)ToolRegistry::Get().Register(                            \
                    NameStr, [](){ return std::make_unique<Type>(); }          \
                );                                                             \
            }                                                                  \
        };                                                                     \
        static AutoToolReg_##Type s_auto_tool_reg_##Type;                      \
    }
