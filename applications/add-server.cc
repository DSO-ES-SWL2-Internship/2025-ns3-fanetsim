#include "add-server.h"
#include "ns3/log.h"

namespace ns3
{
    NS_OBJECT_ENSURE_REGISTERED(AddServer);
    NS_LOG_COMPONENT_DEFINE("AddServer");
 
    void AddServer::HandleRead(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        Address from;
        
        // Keep reading until the socket buffer is empty
        while ((packet = socket->RecvFrom(from))) {
            if (packet->GetSize() == 0) break; // End of messages

            uint32_t data[3];  // Assuming the packet format is correct
            packet->CopyData(reinterpret_cast<uint8_t*>(data), sizeof(data));

            NS_LOG_INFO("Received request from " << data[2] << ": " << data[0] << " + " << data[1] << " = ");
            
            uint32_t sum = data[0] + data[1];

            Ptr<Packet> response = Create<Packet>(reinterpret_cast<uint8_t*>(&sum), sizeof(sum));
            socket->SendTo(response, 0, from);
        }
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