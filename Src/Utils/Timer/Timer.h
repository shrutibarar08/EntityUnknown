#pragma once
#include <chrono>


class Timer
{
public:
    using Clock = std::chrono::high_resolution_clock;

    Timer()
    {
        const auto now = Clock::now();
        m_StartTime = now;
        m_LastTick = now;
    }

    // Resets both start and tick
    void Reset()
    {
        const auto now = Clock::now();
        m_StartTime = now;
        m_LastTick = now;
    }

    // Updates tick time and returns delta time since last tick
    float Tick()
    {
        const auto now = Clock::now();
        std::chrono::duration<float> delta = now - m_LastTick;
        m_LastTick = now;
        return delta.count();
    }

    // Time since start (after reset)
    float Elapsed() const
    {
        using namespace std::chrono;
        return duration<float>(Clock::now() - m_StartTime).count();
    }

    // Has the specified amount of time passed since Reset()?
    bool HasElapsed(float seconds) const
    {
        return Elapsed() >= seconds;
    }

    // Time since last tick (but doesn't update it)
    float Delta() const
    {
        using namespace std::chrono;
        return duration<float>(Clock::now() - m_LastTick).count();
    }

private:
    Clock::time_point m_StartTime;
    Clock::time_point m_LastTick;
};
