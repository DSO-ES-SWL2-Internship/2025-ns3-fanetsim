#include "gdt-app.h"

namespace ns3 
{
    NS_LOG_COMPONENT_DEFINE("GDTApp");

    GDTApp::GDTApp()
        : m_socket(nullptr), m_port(8080)
    {

    }

    GDTApp::~GDTApp()
    {
        if (m_socket)
        {
            m_socket->Close();
            m_socket = nullptr;
        }
    }

    void GDTApp::SetPort(uint16_t port)
    {
        m_port = port;
    }

    void GDTApp::StartApplication()
    {
        NS_LOG_DEBUG("GDT App started on GDT");
    }

    void GDTApp::StopApplication()
    {
        if (m_socket) 
        {
            m_socket->Close();
            m_socket = nullptr;
        }
    }

    void GDTApp::DoInitialize()
    {
        if (!m_socket)
        {
            m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            m_socket->Bind(local);
            m_socket->SetRecvCallback(MakeCallback(&GDTApp::HandleRead, this));
            m_socket->SetAttribute("RcvBufSize", UintegerValue(65536));
        }

        Application::DoInitialize();
    }

    void GDTApp::HandleRead(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        Address from;

        while ((packet = socket->RecvFrom(from)))
        {
            m_packetQueue.push(packet);
            m_addressQueue.push(from);
        }

        if (!m_packetQueue.empty())
        {
            Simulator::ScheduleNow(&GDTApp::ProcessNextPacket, this);
        }
    }

    void GDTApp::ProcessNextPacket()
    {
        if (m_packetQueue.empty())
            return;

        Ptr<Packet> packet = m_packetQueue.front();
        Address addr = m_addressQueue.front();
        m_packetQueue.pop();
        m_addressQueue.pop();

        uint8_t buffer[128] = {0};
        packet->CopyData(buffer, packet->GetSize());

        std::string msg ((char *) buffer);
        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << " GDT received message: " << msg);

        if (!m_packetQueue.empty())
        {
            Simulator::ScheduleNow(&GDTApp::ProcessNextPacket, this);
        }
    }
}