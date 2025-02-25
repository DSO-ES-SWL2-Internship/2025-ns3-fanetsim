#include "add-server.h"

namespace ns3
{
    void AddServer::HandleRead(Ptr<Socket> socket) {
        Address from;
        Ptr<Packet> packet = socket->RecvFrom(from);
        uint32_t data[2];

        packet->CopyData(reinterpret_cast<uint8_t*>(data), sizeof(data));
        uint32_t sum = data[0] + data[1];

        NS_LOG_INFO("Server received: " << data[0] << " + " << data[1] << " = " << sum);

        Ptr<Packet> response = Create<Packet>(reinterpret_cast<uint8_t*>(&sum), sizeof(sum));
        socket->SendTo(response, 0, from);
    }

    void AddServer::StartApplication() {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 8080);
        m_socket->Bind(local);
        m_socket->SetRecvCallback(MakeCallback(&AddServer::HandleRead, this));
    }

    void AddServer::StopApplication() {
        if (m_socket) {
            m_socket->Close();
        }
    }
}