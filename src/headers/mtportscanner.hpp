#pragma once

#include "asio.hpp"

#include "headers/header.hpp"
#include "headers/progressbar.hpp"
#include "headers/scanresultstruct.hpp"
#include "headers/scansession.hpp"
#include "headers/tsmap.hpp"
#include "headers/tsvector.hpp"

#include <iostream>
#include <syncstream>
#include <memory>
#include <functional>
#include <future>

// #define NDEBUG

using namespace std::chrono_literals;
using namespace std::chrono;

class MTPortScanner
{
public:
    static constexpr int THREAD_MULTIPLIER { 25 };
    
    MTPortScanner(
        const std::string& host,
        int startPort,
        int endPort,
        int threadCount,
        const std::chrono::seconds& timeout = std::chrono::seconds(5));

    void setScanFinishedPromise();
    void scanNextPort(std::shared_ptr<ProgressBarStats> pbStats);
    void run();
    void printResults();
    
private:
    asio::io_context m_ioContext;

    std::string m_host;
    int m_startPort;
    int m_endPort;

    asio::ip::tcp::resolver m_resolver;
    asio::ip::tcp::endpoint m_endPoint;
    
    int m_threadCount;
    std::atomic<int> m_currentPort;

    std::chrono::seconds m_timeout;
    asio::executor_work_guard
        <asio::io_context::executor_type> m_workGuard;
    
    std::shared_ptr<TSVector<ScanResult>> m_resultsPtr;
    std::promise<void> m_scanFinishedPromise;
    std::mutex m_mutex;
};