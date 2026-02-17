#include "headers/scanresultstruct.hpp"

std::ostream& operator<<(std::ostream& os, const PortStatus& portStatus)
{
    switch(portStatus)
    {
        case PortStatus::OPEN:
            os << "OPEN";
            break;
        case PortStatus::CLOSE:
            os << "CLOSE";
            break;
        case PortStatus::FILTERED:
            os << "FILTERED";
            break;
    }

    return os;
}

ScanResult::ScanResult(const PortStatus& status, int port)
    : m_status{ status }, m_port{ port }, m_response{ std::nullopt }
{
}

ScanResult::ScanResult(const PortStatus& status, int port, const std::string& response)
    : m_status{ status }, m_port{ port }, m_response{ response }
{
}