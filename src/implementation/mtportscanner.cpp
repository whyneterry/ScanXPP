#include "headers/mtportscanner.hpp"

MTPortScanner::MTPortScanner(
        const std::string& host,
        int startPort,
        int endPort,
        int threadCount,
        const std::chrono::seconds& timeout)
    : m_ioContext{},
    m_host{ host }, m_startPort{ startPort }, m_endPort{ endPort },
    m_resolver{ m_ioContext },
    m_endPoint{},
    m_threadCount{ threadCount },
    m_currentPort{ m_startPort },
    m_timeout{ timeout },
    m_workGuard{ asio::make_work_guard(m_ioContext) },
    m_resultsPtr{ std::make_shared<TSVector<ScanResult>>(m_endPort - m_startPort + 1) },
    m_scanFinishedPromise{}
{
    auto endPoints = m_resolver.resolve(
        m_host,
        std::to_string(m_startPort));

    for(auto& ep : endPoints)
    {
        if(ep.endpoint().address().is_v4())
        {
            m_endPoint = ep;
        }
    }
}

void MTPortScanner::setScanFinishedPromise()
{
    try
    {
        m_scanFinishedPromise.set_value();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void MTPortScanner::scanNextPort(std::shared_ptr<ProgressBarStats> pbStats)
{
    int port = m_currentPort.fetch_add(1);
    if (port > m_endPort)
    {
        return;
    }
    
    auto curEndPoint = m_endPoint;
    curEndPoint.port(port);
    
    ScanSession::create(
        m_ioContext,
        m_host,
        port,
        m_endPort,
        m_timeout,
        m_resultsPtr,
        pbStats,
        [this, pbStats](){ this->scanNextPort(pbStats); } )->start(curEndPoint);
}

void MTPortScanner::run()
{
    #ifdef NDEBUG
        std::osyncstream sync_cout{ std::cout };
    #endif
    auto scanFinishedFuture = m_scanFinishedPromise.get_future();
    auto pbStats =
        ProgressBarStats::create((m_endPort - m_startPort + 1), 0);
    pbStats->setScanFinishHandler([this](){ this->setScanFinishedPromise(); });
    pbStats->threadPrintProgress();

    std::vector<std::jthread> threads;
    threads.reserve(m_threadCount);
    for (int i = 0; i < m_threadCount; ++i)
    {
        threads.emplace_back([this]{ m_ioContext.run(); });
    }

    int limit = m_threadCount * THREAD_MULTIPLIER;
    for (int i = 0; i < limit; ++i)
    {
        scanNextPort(pbStats);
    }

    m_workGuard.reset();
    #ifdef NDEBUG
        std::cout << "reset has been called. now we're waiting for set_value()" << std::endl;
    #endif
    try
    {
        scanFinishedFuture.wait();
        #ifdef NDEBUG
            std::cout << "future got the value" << std::endl;
        #endif
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch(...)
    {
        std::cout << "Unknown exception" << std::endl;
    }
    #ifdef NDEBUG
        std::cout << "end of run. waiting for all jthread to finish" << std::endl;
    #endif
}

void helperPrintResults(ScanResult&& scanResult)
{
    std::cout << "Port: " << scanResult.m_port << '\n';
    std::cout << "Status: " << scanResult.m_status << '\n';
    if(scanResult.m_response.has_value())
    {
        std::cout << "Response:\n" << scanResult.m_response.value() << '\n';
    }
}

void MTPortScanner::printResults()
{
    std::cout << "=====SCAN RESULTS=====\n";
    auto openPortsCopy =
        m_resultsPtr->copyAsShared();
    for(auto& scanResult : *openPortsCopy)
    {
        helperPrintResults(std::move(scanResult));
    }
    std::cout << std::endl;
}