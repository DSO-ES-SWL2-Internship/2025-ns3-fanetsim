#include "add-client.h"
#include "ns3/log.h"


namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("AddClient");
    
    void AddClient::SendPacket() {
        uint32_t nodeId = GetNode()->GetId();
        uint32_t num1 = m_randomValue->GetInteger(1, 100);
        uint32_t num2 = m_randomValue->GetInteger(1, 100);

        uint32_t data[2] = {static_cast<uint32_t>(num1), static_cast<uint32_t>(num2)};
        Ptr<Packet> packet = Create<Packet>(reinterpret_cast<uint8_t*>(data), sizeof(data));
        
        m_socket->Send(packet);

        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
                << nodeId << " sent addition question");

        // Schedule next send at random time
        Time nextTime = Seconds(m_randomTime->GetValue());
        m_sendEvent = Simulator::Schedule(nextTime, &AddClient::SendPacket, this);
    }

    AddClient::AddClient()
    {
        
    }

    void AddClient::Setup(Address peerAddress, uint16_t peerPort) {
        m_peerAddress = peerAddress;
        m_peerPort = peerPort;
        m_randomTime = CreateObject<UniformRandomVariable>();
        m_randomTime->SetAttribute("Min", DoubleValue(1.0));
        m_randomTime->SetAttribute("Max", DoubleValue(5.0));
        m_randomValue = CreateObject<UniformRandomVariable>();
    }

    void AddClient::StartApplication() {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        m_socket->Bind();
        m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));

        SendPacket();  // Start sending packets
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