#include "gdt-app.h"
#include "ns3/fanet-communication.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

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
        //Simulator::Schedule(Seconds(19), &GDTApp::PrintCHTable, this);
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
            RegisterHandlers();
        }

        FANETApplication::DoInitialize();
    }

    void GDTApp::RegisterHandlers()
    {
        FANETApplication::RegisterHandlers();

        helloServiceHandlers[CH_PROMO] = [this] (FANETHeader* header, Ptr<Packet> packet, Address from) { HandleCHPromo(header, packet, from); };
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