#include "fanet-application.h"

namespace ns3 
{
    NS_LOG_COMPONENT_DEFINE("FANETApplication");

    FANETApplication::FANETApplication()
        : m_socket(nullptr), m_destAddr(Ipv4Address("0.0.0.0")), m_port(8080) {}

    FANETApplication::~FANETApplication(){}

    void FANETApplication::SetDestAddr(Ipv4Address destAddr) { m_destAddr = destAddr; }

    void FANETApplication::SetPort(uint16_t port) { m_port = port; }

    Ptr<Socket> FANETApplication::GetSocket() { return m_socket; }

    Ipv4Address FANETApplication::GetDestAddr() { return m_destAddr; }

    uint16_t FANETApplication::GetPort() { return m_port; }

    std::queue<Ptr<Packet>>& FANETApplication::GetPacketQueue() { return m_packetQueue; }

    std::queue<Address>& FANETApplication::GetAddressQueue() { return m_addressQueue;}
}