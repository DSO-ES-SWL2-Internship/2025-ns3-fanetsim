#include "ClusterNodePromotionApp.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("ClusterNodePromotionApp");

    ClusterNodePromotionApp::ClusterNodePromotionApp()
        :m_socket(nullptr), m_gdtIp(Ipv4Address("0.0.0.0")), m_port(8080), m_isClusterHead(false), m_isActive(false)
    {

    }

    ClusterNodePromotionApp::~ClusterNodePromotionApp()
    {
        if (m_socket)
        {
            m_socket->Close();
            m_socket = nullptr;
        }
    }

    void ClusterNodePromotionApp::SetUp(Ipv4Address gdtIp, uint16_t port, uint32_t clusterIndex)
    {
        m_gdtIp = gdtIp;
        m_port = port;
        m_clusterIndex = clusterIndex;
    }

    void ClusterNodePromotionApp::StartApplication()
    {
        if (!m_socket)
        {
            m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            m_socket->Bind(local);
            m_socket->SetRecvCallback(MakeCallback(&ClusterNodePromotionApp::HandleRead, this));
        }

        NS_LOG_DEBUG("Node " << GetNode()->GetId() << "application to notify gdt that it became clusterhead started");
    }

    void ClusterNodePromotionApp::StopApplication()
    {
        if (m_socket)
        {
            m_socket->Close();
            m_socket = nullptr;
        }
    }

    void ClusterNodePromotionApp::NotifyGDT(bool isCH)
    {
        m_isClusterHead = isCH;

        if (isCH)
        {
            if (!m_isActive)
            {
                m_isActive = true;
                Simulator::ScheduleNow(&ClusterNodePromotionApp::SendMessage, this);
                NS_LOG_DEBUG("Node " << GetNode()->GetId() << " scheduled to notify GDT of its promotion to CH");
            }
            else
            {
                if (m_isActive)
                {
                    m_isActive = false;
                    NS_LOG_DEBUG("Node " << GetNode()->GetId() << " demoted back to CH");
                }
            }
        }        
    }

    void ClusterNodePromotionApp::SendMessage()
    {
        if (!m_isClusterHead || !m_isActive)
            return;

        std::ostringstream message;
        message << "Node " << GetNode()->GetId() << "is now cluster " << m_clusterIndex << " CH";

        Ptr<Packet> packet = Create<Packet>((uint8_t*) message.str().c_str(), message.str().length());

        m_socket->Send(packet);

        NS_LOG_DEBUG("Node " << GetNode()->GetId() << "application sent GDT the notification");        
    }

    void ClusterNodePromotionApp::HandleRead(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        Address from;

        while ((packet = socket->RecvFrom(from)))
        {
            uint8_t buffer [128] = {0};
            packet->CopyData(buffer, packet->GetSize());

            std::string msg((char *) buffer);

            NS_LOG_DEBUG("Node " << GetNode()->GetId() << " application received CH status notification");

            if (msg == "BECOME_CH")
            {
                NotifyGDT(true);
            }
            else if (msg == "STOP_CH")
            {
                NotifyGDT(false);
            }
        }
    }
}