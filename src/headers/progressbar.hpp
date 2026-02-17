#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <iostream>
#include <iomanip>
#include <syncstream>
#include <chrono>
#include <future>

using namespace std::chrono_literals;

class ProgressBarStats : public std::enable_shared_from_this<ProgressBarStats>
{
public:
    using ScanFinishHandler = std::function<void()>;

    ~ProgressBarStats();
    ProgressBarStats(
        int portsCount,
        int scannedPortsCount);

    static std::shared_ptr<ProgressBarStats> create(
        int portsCount,
        int scannedPortsCount);

    void setScanFinishHandler(ScanFinishHandler scanFinish);
        
    void incrementScannedPortsCount();

    void printProgress();

    void threadPrintProgress();

    void stop();

    private:
        const int m_portsCount;
        std::atomic<int> m_scannedPortsCount;
        std::atomic<bool> m_scanning{ true };
        std::unique_ptr<std::thread> m_progressThread;

        ScanFinishHandler m_scanFinish;
};