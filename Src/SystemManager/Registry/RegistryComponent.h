#pragma once

#include <functional>
#include <string_view>
#include <unordered_map>
#include <any>

#include "Utils/HelperFunctions.h"
#include "ExceptionManager/IException.h"

class LevelEditorContext;

struct ComponentInfo
{
    std::string_view name;
    std::function<void* (void)>                       create;         // instance, owned by editor
    std::function<void(void*, LevelEditorContext&)>   drawInspector; // UI
    std::function<void(void*, LevelEditorContext&)>   serialize;     // storage policy
    std::function<void* (void*, LevelEditorContext&)> clone;         // deep copy
};

class RegistryComponent
{
public:
    static RegistryComponent& Get()
    {
        static RegistryComponent instance;
        return instance;
    }

    void Register(ComponentInfo info)
    {
        const auto key = ct_hash(info.name.data());
        m_components.try_emplace(key, std::move(info));
    }

    const ComponentInfo* Find(std::string_view str) const
    {
        auto it = m_components.find(ct_hash(str.data()));
        return (it == m_components.end()) ? nullptr : &it->second;
    }

    // Optional: direct lookup by key if you ever store the hash on entities
    const ComponentInfo* FindByHash(std::uint64_t key) const
    {
        auto it = m_components.find(key);
        return (it == m_components.end()) ? nullptr : &it->second;
    }

    const auto& GetAll() const { return m_components; }

private:
    std::unordered_map<uint64_t, ComponentInfo> m_components;
};

// ---------- FIXED MACRO ----------
#define REGISTER_COMPONENT(Type, NameStr, CreateFn, DrawFn, SerFn, CloneFn)     \
    namespace {                                                                 \
        struct AutoReg_##Type {                                                 \
            AutoReg_##Type() {                                                  \
                RegistryComponent::Get().Register(ComponentInfo{                \
                    NameStr, CreateFn, DrawFn, SerFn, CloneFn                   \
                });                                                             \
            }                                                                   \
        };                                                                      \
        static AutoReg_##Type s_autoreg_##Type;                                 \
    }
