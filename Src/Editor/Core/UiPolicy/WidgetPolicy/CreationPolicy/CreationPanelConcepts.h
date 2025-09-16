#pragma once
#include <vector>

// Require: nested Item type, and both Register overloads.
template<class T>
concept HasCreationRegister = requires(T t,
    const typename T::Item & one,
    const std::vector<typename T::Item>&many)
{
    t.Register(one);
    t.Register(many);
};

// Optional: sanity check that the panel exposes DrawCreation(LevelEditorContext*)
class LevelEditorContext;
template<class T>
concept HasDrawCreation = requires(T t, LevelEditorContext* c) {
    t.DrawCreation(c);
};
