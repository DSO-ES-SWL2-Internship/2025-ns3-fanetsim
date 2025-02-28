#include "add-client.h"
#include "ns3/log.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("AddClient");
    
    void AddClient::SendPacket() {
        uint32_t nodeId = GetNode()->GetId();
        uint32_t num1 = m_randomValue->GetInteger(1, 100);
        uint32_t num2 = m_randomValue->GetInteger(1, 100);

        uint32_t data[3] = {num1, num2, nodeId};
        Ptr<Packet> packet = Create<Packet>(reinterpret_cast<uint8_t*>(data), sizeof(data));
        packet->AddPaddingAtEnd(sizeof(data));
        
        m_socket->Send(packet);

        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
                << nodeId << " scheduled to sent addition question: " << num1 << " + " << num2);

        double randomInterval = m_randomValue->GetValue(1.0, 5.0);

        // Schedule next send in exactly 5 seconds
        m_sendEvent = Simulator::Schedule(Seconds(1), &AddClient::SendPacket, this);
    }

    void AddClient::HandleRead(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        Address from;
        while ((packet = socket->RecvFrom(from))) {
            if (packet->GetSize() == sizeof(uint32_t)) {
                uint32_t result;
                packet->CopyData(reinterpret_cast<uint8_t*>(&result), sizeof(uint32_t));

                NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node "
                        << GetNode()->GetId() << " received result: " << result);
            }
        }
    }

    AddClient::AddClient() {}

    void AddClient::Setup(Address peerAddress, uint16_t peerPort) {
        m_peerAddress = peerAddress;
        m_peerPort = peerPort;
        m_randomValue = CreateObject<UniformRandomVariable>();
    }

    void AddClient::StartApplication() {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        //m_socket->Bind();
        m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 50000 + GetNode()->GetId()));

        m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));

        // Set receive callback to handle incoming packets
        m_socket->SetRecvCallback(MakeCallback(&AddClient::HandleRead, this));

        SendPacket();  // Start sending packets every 5 seconds
    }

    void AddClient::StopApplication() {
        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }
        if (m_socket) {
            m_socket->Close();
        }
    }
}
