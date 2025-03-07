#include "fanet-application.h"
#include "ns3/core-module.h"

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

    void FANETApplication::RegisterHelloHandler(int serviceType, std::function<void(FANETHeader*, Ptr<Packet>)> handler) {
        helloServiceHandlers[serviceType] = handler;
    }

    void FANETApplication::RegisterDataHandler(int serviceType, std::function<void(FANETHeader*, Ptr<Packet>)> handler) {
        dataServiceHandlers[serviceType] = handler;
    }

    void FANETApplication::RegisterRequestHandler(int serviceType, std::function<void(FANETHeader*, Ptr<Packet>)> handler) {
        requestServiceHandlers[serviceType] = handler;
    }

    void FANETApplication::RegisterResponseHandler(int serviceType, std::function<void(FANETHeader*, Ptr<Packet>)> handler) {
        responseServiceHandlers[serviceType] = handler;
    }

    void FANETApplication::ProcessNextPacket()
    {
        if (m_packetQueue.empty())
            return;

        Ptr<Packet> packet = m_packetQueue.front();
        Address addr = m_addressQueue.front();
        m_packetQueue.pop();
        m_addressQueue.pop();

        FANETHeader header;
        packet->RemoveHeader(header);

        // Use the dynamically registered handlers
        switch (header.GetType())
        {
            case HELLO:
                ProcessHelloPacket(&header, packet);
                break;
            case DATA:
                ProcessDataPacket(&header, packet);
                break;
            case REQUEST:
                ProcessRequestPacket(&header, packet);
                break;
            case RESPONSE:
                ProcessResponsePacket(&header, packet);
                break;
            default:
                NS_LOG_WARN("Unknown packet type received!");
                return;
        }

        if (!m_packetQueue.empty()) {
            Simulator::ScheduleNow(&FANETApplication::ProcessNextPacket, this);
        }
    }

    void FANETApplication::ProcessHelloPacket(FANETHeader* header, Ptr<Packet> packet) {
        auto it = helloServiceHandlers.find(header->GetService());
        if (it != helloServiceHandlers.end()) {
            it->second(header, packet);  
        } else {
            NS_LOG_UNCOND("No Hello handler registered for service type: " << header->GetService());
        }
    }

    void FANETApplication::ProcessDataPacket(FANETHeader* header, Ptr<Packet> packet) {
        auto it = dataServiceHandlers.find(header->GetService());
        if (it != dataServiceHandlers.end()) {
            it->second(header, packet);
        } else {
            NS_LOG_INFO("No Data handler registered for service type: " << header->GetService());
        }
    }

    void FANETApplication::ProcessRequestPacket(FANETHeader* header, Ptr<Packet> packet) {
        auto it = requestServiceHandlers.find(header->GetService());
        if (it != requestServiceHandlers.end()) {
            it->second(header, packet);
        } else {
            NS_LOG_INFO("No Request handler registered for service type: " << header->GetService());
        }
    }

    void FANETApplication::ProcessResponsePacket(FANETHeader* header, Ptr<Packet> packet) {
        auto it = responseServiceHandlers.find(header->GetService());
        if (it != responseServiceHandlers.end()) {
            it->second(header, packet);
        } else {
            NS_LOG_INFO("No Response handler registered for service type: " << header->GetService());
        }
    }
}