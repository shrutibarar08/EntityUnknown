#include "EventQueue.h"

void EventBus::DispatchAll()
{
    std::ranges::sort(s_EventQueue.begin(), s_EventQueue.end(), [](const Event& a, const Event& b)
        {
            return a.priority > b.priority;
        });

    for (const Event& evt : s_EventQueue)
    {
        auto it = s_Listeners.find(evt.type);
        if (it != s_Listeners.end())
        {
            for (auto& func : it->second)
            {
                func(evt.data);
            }
        }
    }

    s_EventQueue.clear();
}