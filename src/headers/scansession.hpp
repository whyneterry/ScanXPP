#pragma once

#include "asio.hpp"

#include "headers/header.hpp"
#include "headers/progressbar.hpp"
#include "headers/scanresultstruct.hpp"
#include "headers/timeanalyzer.hpp"
#include "headers/tsmap.hpp"
#include "headers/tsvector.hpp"

#include <iostream>
#include <syncstream>
#include <cstdlib>
#include <memory>
#include <functional>
#include <future>

// #define NDEBUG

class ScanSession : public std::enable_shared_from_this<ScanSession>
{
public:
    static constexpr size_t READBUF_SIZE{ 1024 };

    using EndPoints = asio::ip::tcp::resolver::results_type;
    using EndPoint = asio::ip::tcp::endpoint;
    using Socket = asio::ip::tcp::socket;
    using Timer = asio::steady_timer;
    using ProgressBarPtr = std::shared_ptr<ProgressBarStats>;
    using ResultsPtr = std::shared_ptr<TSVector<ScanResult>>;
    using ScanNextHandler = std::function<void()>;
    using Buffer = asio::streambuf;

    ScanSession(
        asio::io_context& io,
        std::string_view host,
        int port,
        int endPort,
        const std::chrono::seconds& timeout,
        ResultsPtr ResultsPtr,
        ProgressBarPtr pbStatsPtr,
        ScanNextHandler scanNext);

    static std::shared_ptr<ScanSession> create(
        asio::io_context& io,
        std::string_view m_host,
        int port,
        int endPort,
        const std::chrono::seconds& timeout,
        ResultsPtr ResultsPtr,
        ProgressBarPtr pbStatsPtr,
        ScanNextHandler scanNext);

    void setTimerFlag();
    void incrementScannedPortsCount();
    void registerCurrentTimePoint();
    std::chrono::steady_clock::duration howMuchElapsed();

    Buffer& getReadBuffer();
    Buffer* getReadBufferPtr();
    void openPortsEmplaceBack(ScanResult&& scanResult);
    
    void closeSocket();
    void cancelTimer();
    void setTimer();
    void resetTimer();

    std::string extractCodeAndServerHeaders(std::string& response);

    int getPort();
    bool getTimerFlag();

    void tryHttpGet();
    void trySsh();
    void readHttpResponse();
    void readSshResponse();
    void closeConnection();

    void onTimeout(const asio::error_code& ec);
    void onConnect(const asio::error_code& ec);

    void start(const EndPoint endPoint);

private:
    std::string_view m_host;
    int m_port;
    int m_endPort;

    std::chrono::seconds m_timeout;

    Socket m_socket;
    Timer m_timer;
    std::atomic<bool> m_timerFlag;
    ResultsPtr m_resultsPtr; 

    ProgressBarPtr m_pbStatsPtr;

    TimeAnalyzer m_timeAnalyzer;

    std::function<void()> m_scanNext;

    std::atomic<bool> m_oneCloseCallFlag;

    Buffer m_readBuffer;
};