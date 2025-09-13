#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "RenderManager/Interface/IRender.h"


class RegistryMesh
{
public:

    using CreateFunc = std::function<std::unique_ptr<IRender>()>;

    static void Register(const std::string& name, CreateFunc createFunc)
    {
        if (registry_.contains(name)) return;

        registry_[name] = std::move(createFunc);
        mNames.push_back(name);
    }
    static std::unique_ptr<IRender> Create(const std::string& name)
    {
        auto it = registry_.find(name);
        return it != registry_.end() ? it->second() : nullptr;
    }
    static std::vector<std::string>& GetRegisteredNames()
    {
        return mNames;
    }

private:
    inline static std::unordered_map<std::string, CreateFunc> registry_;
    inline static std::vector<std::string> mNames;
};

#define REGISTER_MESH(CLASS_NAME) \
    namespace { \
        struct CLASS_NAME##Registrar { \
            CLASS_NAME##Registrar() { \
                RegistryMesh::Register(#CLASS_NAME, []() { \
                    auto obj = std::make_unique<CLASS_NAME>(); \
                    obj->SetTypeName(#CLASS_NAME); \
                    return obj; \
                }); \
            } \
        }; \
        static CLASS_NAME##Registrar CLASS_NAME##_registrar; \
    }
