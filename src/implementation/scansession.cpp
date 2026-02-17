#include "headers/scansession.hpp"

ScanSession::ScanSession(
    asio::io_context& io,
    std::string_view host,
    int port,
    int endPort,
    const std::chrono::seconds& timeout,
    ScanSession::ResultsPtr resultsPtr,
    ScanSession::ProgressBarPtr pbStatsPtr,
    ScanSession::ScanNextHandler scanNext
    )
    : m_host{ host },
    m_port{ port },
    m_endPort{ endPort },
    m_timeout{ timeout },
    m_socket{ io },
    m_timer{ io, m_timeout },
    m_timerFlag{ false },
    m_resultsPtr{ resultsPtr },
    m_pbStatsPtr{ pbStatsPtr },
    m_timeAnalyzer{},
    m_scanNext{ scanNext },
    m_oneCloseCallFlag{ false }
{
}

std::shared_ptr<ScanSession> ScanSession::create(
    asio::io_context& io,
    std::string_view host,
    int port,
    int endPort,
    const std::chrono::seconds& timeout,
    ScanSession::ResultsPtr resultsPtr,
    ScanSession::ProgressBarPtr pbStatsPtr,
    ScanSession::ScanNextHandler scanNext
    )
{
    return std::make_shared<ScanSession>(io, host, port, endPort, timeout, resultsPtr, pbStatsPtr, scanNext);
}

void ScanSession::setTimerFlag()
{
    auto expected = m_timerFlag.load();
    while(!m_timerFlag.compare_exchange_strong(expected, true))
    {
    }
}

void ScanSession::incrementScannedPortsCount()
{
    m_pbStatsPtr->incrementScannedPortsCount();
}

void ScanSession::registerCurrentTimePoint()
{
    m_timeAnalyzer.registerCurrentTimePoint();
}

std::chrono::steady_clock::duration ScanSession::howMuchElapsed()
{
    return m_timeAnalyzer.howMuchElapsed();
}

ScanSession::Buffer& ScanSession::getReadBuffer()
{
    return m_readBuffer;
}

ScanSession::Buffer* ScanSession::getReadBufferPtr()
{
    return &m_readBuffer;
}

void ScanSession::openPortsEmplaceBack(ScanResult&& scanResult)
{
    m_resultsPtr->emplace_back(std::forward<ScanResult>(scanResult));
}

void ScanSession::closeSocket()
{
    m_socket.close();
}

void ScanSession::cancelTimer()
{
    m_timer.cancel();
}

void ScanSession::setTimer()
{
    m_timer.async_wait(
        [self = shared_from_this()](const asio::error_code& ec)
        {
            self->onTimeout(ec);
        });
}

void ScanSession::resetTimer()
{
    m_timer.expires_after(m_timeout);
    setTimer();
}

void validateIndexThrow(size_t index, const char* exceptionMessage)
{
    if (index == std::string::npos)
    {
        throw std::invalid_argument(exceptionMessage);
    }
}

std::string ScanSession::extractCodeAndServerHeaders(std::string& response)
{
    size_t codeIndexStart = response.find("HTTP/");
    validateIndexThrow(codeIndexStart, "There is no HTTP Header in response");

    size_t codeIndexEnd = response.find("\r\n", codeIndexStart);
    validateIndexThrow(codeIndexEnd, "There is no end of HTTP Header in response.");

    size_t servertIndexStart = response.find("Server:");
    validateIndexThrow(servertIndexStart, "There is no Server Header in response.");

    size_t servertIndexEnd = response.find("\r\n", servertIndexStart);
    validateIndexThrow(servertIndexEnd, "There is no end of Server Header in response.");

    std::string extractedHeaders{
        response.substr(codeIndexStart, codeIndexEnd - codeIndexStart)
        +
        "\n" +
        response.substr(servertIndexStart,  servertIndexEnd - servertIndexStart) };

    return extractedHeaders;
}

int ScanSession::getPort()
{
    return m_port;
}

bool ScanSession::getTimerFlag()
{
    return m_timerFlag.load();
}

void ScanSession::tryHttpGet()
{
    std::string request = std::format(
        "GET / HTTP/1.1\r\n"
        "Host: {}\r\n"
        "Connection: close\r\n\r\n", m_host);

    asio::async_write(m_socket, asio::buffer(request.data(), request.size()),
        [self = shared_from_this()](const asio::error_code& ec, size_t /*len*/)
        {
            if (!ec)
            {
                self->readHttpResponse();
            }
            else
            {
                self->closeConnection();
            }
        });
}

void ScanSession::trySsh()
{
    /* Some preparation*/
    readSshResponse();
}

void ScanSession::readSshResponse()
{
    resetTimer();

    asio::async_read_until(m_socket, m_readBuffer, "\n",
        [self = shared_from_this()](const asio::error_code& ec, size_t len)
        {
            if (!ec)
            {
                std::string response{ std::istreambuf_iterator<char>(self->getReadBufferPtr()), std::istreambuf_iterator<char>() };
                self->openPortsEmplaceBack({PortStatus::OPEN, self->getPort(), response});
            }
            else
            {
                std::cout << ec.message() << std::endl;
            }
            self->closeConnection();
        });
}

void ScanSession::readHttpResponse()
{
    resetTimer();

    asio::async_read_until(m_socket, m_readBuffer, "\r\n\r\n",
        [self = shared_from_this()](const asio::error_code& ec, size_t /*len*/)
        {
            if(!ec)
            {
                std::string response{ std::istreambuf_iterator<char>(self->getReadBufferPtr()), std::istreambuf_iterator<char>() };
                try
                {
                    auto extractedHeaders = self->extractCodeAndServerHeaders(response);
                    self->openPortsEmplaceBack({PortStatus::OPEN, self->getPort(), extractedHeaders});
                }
                catch(const std::invalid_argument& /*e*/)
                {
                    std::cout << "ERROR. extractCodeAndServerHeaders\n";
                    // std::cout << e.message() << std::endl;
                }
            }
            else
            {
                std::cout << "ERROR. async_read_until\n";
                // std::cout << ec.message() << std::endl;
            }
            self->closeConnection();
        });
}

void ScanSession::closeConnection()
{
    bool expected = false;
    if(!m_oneCloseCallFlag.compare_exchange_strong(expected, true))
    {
        return;
    }
    cancelTimer();
    closeSocket();
    incrementScannedPortsCount();
    m_scanNext();
}

void ScanSession::onTimeout(const asio::error_code& ec)
{
    if (ec == asio::error::operation_aborted) // Timer canceled
    {
        return;
    }

    if (!ec) // Timer expired
    {
        #ifdef NDEBUG
            printPortScanTime(getPort(),
                howMuchElapsed());
        #endif

        setTimerFlag();
        m_resultsPtr->emplace_back(ScanResult{PortStatus::FILTERED, getPort()});
        closeConnection();
        
        #ifdef NDEBUG
            std::osyncstream sync_cout{ std::cout };
            sync_cout << "[+-] Port " << getPort() << " is FILTERED\n";
        #endif
    }
}

void ScanSession::onConnect(const asio::error_code& ec)
{
    if (getTimerFlag())
    {
        return;
    }

    #ifdef NDEBUG
        printPortScanTime(getPort(),
            howMuchElapsed());
    #endif

    #ifdef NDEBUG
        std::osyncstream sync_cout{ std::cout };
    #endif

    if (!ec)
    {
        #ifdef NDEBUG
            sync_cout << "[+] Port " << getPort() << " is OPEN\n";
        #endif

        switch(m_port)
        {
        case 22: // SSH
            trySsh();
            break;
        case 80: // HTTP
            tryHttpGet();
            break;
        default: // Default handling
            m_resultsPtr->emplace_back(ScanResult{PortStatus::OPEN, getPort()});
            closeConnection();
        }
    }
    else
    {
        m_resultsPtr->emplace_back(ScanResult{PortStatus::CLOSE, getPort()});
        closeConnection();
        #ifdef NDEBUG
            sync_cout << "[-] Port " << getPort() << " is CLOSED\n";
            sync_cout << ec.message() << std::endl;
        #endif
    }
}

void ScanSession::start(const ScanSession::EndPoint endPoint)
{
    registerCurrentTimePoint();

    setTimer();

    m_socket.async_connect(endPoint,
        [self = shared_from_this()](const asio::error_code& ec)
        {
            self->onConnect(ec);
        });
}