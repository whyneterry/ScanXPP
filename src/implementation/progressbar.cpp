#include "headers/progressbar.hpp"

ProgressBarStats::~ProgressBarStats()
{
    printProgress();
}

std::shared_ptr<ProgressBarStats> ProgressBarStats::create(
    int portsCount,
    int scannedPortsCount)
{
    return std::make_shared<ProgressBarStats>(portsCount, scannedPortsCount);
}

void ProgressBarStats::setScanFinishHandler(ScanFinishHandler scanFinish)
{
    m_scanFinish = scanFinish;
}

ProgressBarStats::ProgressBarStats(
    int portsCount,
    int scannedPortsCount)
    : m_portsCount{ portsCount },
    m_scannedPortsCount{ scannedPortsCount }
{
}

void ProgressBarStats::incrementScannedPortsCount()
{
    int lastScannedPort = m_scannedPortsCount.fetch_add(1) + 1;
    if (lastScannedPort == m_portsCount && m_scanFinish)
    {
        m_scanFinish();
    }
}

void ProgressBarStats::printProgress()
{
    std::osyncstream sync_cout{ std::cout };
    
    int scanned = m_scannedPortsCount.load();
    double percentage{ static_cast<double>(scanned) * 100
        / m_portsCount };

    #ifdef NDEBUG
        std::cout << "DEBUG. Percentage = " << percentage << std::endl;            
    #endif

    std::cout << '\r';
    if (percentage < 100)
    {
        std::cout << "Progress: " << std::fixed << std::setprecision(3) << percentage << "%";
    }
    else
    {
        std::cout << "Finished: " << std::fixed << std::setprecision(3) << percentage << "%\n";
        stop();
    }
}

void ProgressBarStats::threadPrintProgress()
{
    m_progressThread = std::make_unique<std::thread>(
        [this]()
        {
            while(m_scanning.load())
            {
                printProgress();
                std::this_thread::sleep_for(1000ms);
            }
        });
}

void ProgressBarStats::stop()
{
    bool expected = m_scanning.load();
    m_scanning.compare_exchange_strong(expected, false);
    if(m_progressThread->joinable())
    {
        m_progressThread->join();
    }
}