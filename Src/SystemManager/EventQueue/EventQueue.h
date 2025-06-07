#pragma once
#include <any>
#include <cstdint>
#include <functional>
#include <ranges>
#include <algorithm>

#include "Utils/Logger/Logger.h"

enum class EventType: uint16_t
{
	FullScreen,
	WindowedScreen,
	WindowResize,
};

struct FullScreenPayload { UINT width;  UINT height; };
struct WindowedScreenPayload { UINT width; UINT height; };
struct WindowResizePayload { UINT width; UINT height; };

struct Event
{
	EventType type;
	std::any data;	// payload (could be empty for events with no data)
	int priority;
};

class EventBus
{
public:
    template<typename PayloadT>
    static void Subscribe(EventType eventType, std::function<void(const PayloadT&)> callback);

    template<typename PayloadT>
    static void Enqueue(EventType eventType, PayloadT&& payload, int priority = 0);

    static void DispatchAll();

private:
    static inline std::unordered_map<EventType, std::vector<std::function<void(const std::any&)>>> s_Listeners;
    static inline std::vector<Event> s_EventQueue;
};

template<typename PayloadT>
inline void EventBus::Subscribe(EventType eventType, std::function<void(const PayloadT&)> callback)
{
    std::function<void(const std::any&)> wrapper = [func = std::move(callback)](const std::any& payload)
        {
            try
            {
                const PayloadT& dataRef = std::any_cast<const PayloadT&>(payload);
                func(dataRef);
            }
            catch (const std::bad_any_cast&)
            {
                LOG_INFO("[EventBus] Warning: payload type mismatch for event.");
            }
        };
    s_Listeners[eventType].push_back(std::move(wrapper));
}

template<typename PayloadT>
inline void EventBus::Enqueue(EventType eventType, PayloadT&& payload, int priority)
{
    Event evt;
    evt.type = eventType;
    evt.data = std::any(std::forward<PayloadT>(payload));
    evt.priority = priority;
    s_EventQueue.push_back(std::move(evt));
}