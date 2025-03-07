#include "cluster-node-app.h"
#include "ns3/fanet-communication.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("ClusterNodeApp");

    ClusterNodeApp::ClusterNodeApp()
        : m_isClusterHead(false)
    {

    }

    ClusterNodeApp::~ClusterNodeApp()
    {
        if (m_socket)
        {
            m_socket->Close();
            m_socket = nullptr;
        }
    }

    void ClusterNodeApp::SetUp(Ipv4Address gdtIp, uint16_t port, uint32_t clusterIndex)
    {
        m_destAddr = gdtIp;
        m_port = port;
        m_clusterIndex = clusterIndex;
    }

    void ClusterNodeApp::StartApplication()
    {

        NS_LOG_DEBUG("Node " << GetNode()->GetId() << " application to notify gdt that it became clusterhead started");
    }

    void ClusterNodeApp::StopApplication()
    {
        if (m_socket)
        {
            m_socket->Close();
            m_socket = nullptr;
        }
    }

    void ClusterNodeApp::DoInitialize()
    {
        if (!m_socket)
        {
            m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            m_socket->Bind(local);
            m_socket->SetRecvCallback(MakeCallback(&ClusterNodeApp::HandleRead, this));
        }

        Application::DoInitialize();
    }

    void ClusterNodeApp::NotifyGDT(bool isCH)
    {
        m_isClusterHead = isCH;

        if (isCH)
        {
            Simulator::ScheduleNow(&ClusterNodeApp::SendNotifyGdtMessage, this);
            //SendMessage();
            // NS_LOG_DEBUG("Node " << GetNode()->GetId() << " scheduled to notify GDT of its promotion to CH");

        }     
        else
        {
            NS_LOG_DEBUG("Node " << GetNode()->GetId() << " demoted back to CH");
        }
    }

    void ClusterNodeApp::SendNotifyGdtMessage()
    {
        FANETHeader header;
        header.SetType(HELLO);
        header.SetClusterId(m_clusterIndex);
        header.SetNodeId(GetNode()->GetId());
        header.SetService(CH_PROMO);

        std::ostringstream message;
        message << "PROMOTED";


        Ptr<Packet> packet = Create<Packet>((uint8_t*) message.str().c_str(), message.str().length());

        packet->AddHeader(header);

        if (FANETCommunication::SendPacket(this, packet) > 0)
        {
            NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() 
                << " Cluster " << m_clusterIndex 
                << " Node " << GetNode()->GetId() 
                << " scheduled to notify GDT of its promotion to CH");        
        }
        else
        {
            NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() 
                << " Cluster " << m_clusterIndex 
                << " Node " << GetNode()->GetId() 
                << " failed to notify GDT of its promotion to CH");     
        }
    }

    void ClusterNodeApp::SendMessage()
    {
        if (!m_isClusterHead)
            return;


        std::ostringstream message;
        message << "Node " << GetNode()->GetId() << " is now cluster " << m_clusterIndex << " CH";

        Ptr<Packet> packet = Create<Packet>((uint8_t*) message.str().c_str(), message.str().length());

        if (FANETCommunication::SendPacket(this, packet) > 0)
        {
            NS_LOG_DEBUG("At time " << Simulator::Now().GetSeconds() << " Cluster " << m_clusterIndex 
                << " Node " << GetNode()->GetId() << " scheduled to notify GDT of its promotion to CH");        
        }
        else
        {
            NS_LOG_DEBUG("Failed to schedule packet to notify GDT of CH promotion");
        }
    }

    void ClusterNodeApp::HandleRead(Ptr<Socket> socket)
    {
        FANETCommunication::ReceivePacket(this, socket);
    }

    void ClusterNodeApp::ProcessNextPacket()
    {
        if (m_packetQueue.empty())
        {
            return;
        }
        
        Ptr<Packet> packet = m_packetQueue.front();
        Address addr = m_addressQueue.front();
        m_packetQueue.pop();
        m_addressQueue.pop();

        uint8_t buffer[128] = {0};  
        packet->CopyData(buffer, packet->GetSize());

        std::string msg((char*)buffer);

        NS_LOG_DEBUG("Node " << GetNode()->GetId() << " application received CH status notification: " << msg);

        if (msg == "BECOME_CH")
        {
            NotifyGDT(true);
        }
        else if (msg == "STOP_CH")
        {
            NotifyGDT(false);
            m_isClusterHead = false;
        }
    }

    void ClusterNodeApp::ProcessHelloPacket(FANETHeader* header, Ptr<Packet> packet)
    {

    }

    void ClusterNodeApp::ProcessDataPacket(FANETHeader* header, Ptr<Packet> packet)
    {

    }

    void ClusterNodeApp::ProcessRequestPacket(FANETHeader* header, Ptr<Packet> packet)
    {

    }

    void ClusterNodeApp::ProcessResponsePacket(FANETHeader* header, Ptr<Packet> packet)
    {

    }


    void UdpSendTrace(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
    {
        *stream->GetStream() << "Packet Sent: " << packet->GetSize()
                        << " bytes at time " << Simulator::Now().GetSeconds()
                        << "s" << std::endl;
    }

    void ClusterNodeApp::EnableAsciiTracing(Ptr<OutputStreamWrapper> stream)
    {
        m_socket->TraceConnectWithoutContext("Send", MakeBoundCallback(&UdpSendTrace, stream));
    }

    void ClusterNodeApp::EnableInfoLog()
    {
        LogComponentEnable("ClusterNodeApp", LOG_LEVEL_INFO);
    }

    void ClusterNodeApp::EnableDebugLog()
    {
        LogComponentEnable("ClusterNodeApp", LOG_LEVEL_DEBUG);
    }
}