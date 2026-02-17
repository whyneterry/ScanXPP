#pragma once

#include <chrono>

class TimeAnalyzer
{
public:
    void registerCurrentTimePoint();
    const std::chrono::steady_clock::time_point&
        getStartTimePoint() const;
    std::chrono::steady_clock::duration howMuchElapsed() const;

private:
    std::chrono::steady_clock::time_point m_startTimePoint;
};