#include "gdt-app.h"

namespace ns3 
{
    NS_LOG_COMPONENT_DEFINE("GDTApp");

    GDTApp::GDTApp()
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
        Simulator::Schedule(Seconds(19), &GDTApp::PrintCHTable, this);
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
            RegisterHandlers();
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

    void GDTApp::RegisterHandlers()
    {
        helloServiceHandlers[CH_PROMO] = [this] (FANETHeader* header, Ptr<Packet> packet) { HandleCHPromo(header, packet); };
    }

    void GDTApp::EnableInfoLog()
    {
        LogComponentEnable("GDTApp", LOG_LEVEL_INFO);
        LogComponentDisable("GDTApp", LOG_LEVEL_DEBUG);
        LogComponentEnable("GdtChPromo", LOG_LEVEL_INFO);
    }

    void GDTApp::EnableDebugLog()
    {
        LogComponentEnable("GDTApp", LOG_LEVEL_DEBUG);
        LogComponentEnable("GdtChPromo", LOG_LEVEL_DEBUG);
    }
}