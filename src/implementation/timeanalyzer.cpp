#include "headers/timeanalyzer.hpp"

void TimeAnalyzer::registerCurrentTimePoint()
{
    m_startTimePoint = std::chrono::steady_clock::now();
}

const std::chrono::steady_clock::time_point&
    TimeAnalyzer::getStartTimePoint() const
{
    return m_startTimePoint;
}

std::chrono::steady_clock::duration TimeAnalyzer::howMuchElapsed() const
{
    return std::chrono::steady_clock::now() - m_startTimePoint;
}