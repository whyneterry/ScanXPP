#pragma once

#include <iostream>
#include <string>
#include <optional>

enum class PortStatus
{
    OPEN,
    CLOSE,
    FILTERED
};

std::ostream& operator<<(std::ostream& os, const PortStatus& portStatus);

struct ScanResult
{
    ScanResult(const PortStatus& status, int port);
    ScanResult(const PortStatus& status, int port, const std::string& response);

    PortStatus m_status;
    int m_port;
    std::optional<std::string> m_response;
};