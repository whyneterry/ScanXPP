#pragma once

#include <iostream>
#include <syncstream>
#include <chrono>

using namespace std::chrono;

template <typename TimePrecision = std::milli>
void printPortScanTime(int port,
    const steady_clock::duration& time)
{
    std::osyncstream sync_cout{ std::cout };
    sync_cout << "Port " << port <<
        " took " <<
        duration_cast<duration<long long, TimePrecision>>(time)
        << " to scan." << std::endl;
}