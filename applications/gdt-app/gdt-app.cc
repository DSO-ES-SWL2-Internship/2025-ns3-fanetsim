#include "gdt-app.h"
#include "ns3/FANETHeader.h"

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

        FANETHeader header;
        packet->RemoveHeader(header);

        switch (header.GetType())
        {
            case HELLO: {
                ProcessHelloPacket(&header, packet);
                break;
            }

            case DATA: {
                ProcessDataPacket(&header, packet);
                break;
            }

            case REQUEST: {
                ProcessRequestPacket(&header, packet);
                break;
            }

            case RESPONSE: {
                ProcessResponsePacket(&header, packet);
                break;
            }

            default: {

            }
        }

        if (!m_packetQueue.empty())
        {
            Simulator::ScheduleNow(&GDTApp::ProcessNextPacket, this);
        }
    }

    void GDTApp::ProcessHelloPacket(FANETHeader* header, Ptr<Packet> packet)
    {
        switch (header->GetService())
        {
            case GENERAL: {
                break;
            }

            case PLR: {
                break;
            }

            case CH_PROMO: {
                uint8_t buffer[128] = {0};
                packet->CopyData(buffer, 8);

                std::string msg ((char *) buffer );

                NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() 
                                << ", GDT received notification from Cluster " << header->GetClusterId() 
                                << " Node " << header->GetNodeId() 
                                << " that it " << msg << " to CH");
                break;
            }

            case OTHER_SERVICE: {
                break;
            }

            default: {

            }
        }
    }

    void GDTApp::ProcessDataPacket(FANETHeader* header, Ptr<Packet> packet)
    {
        
    }

    void GDTApp::ProcessRequestPacket(FANETHeader* header, Ptr<Packet> packet)
    {
        
    }

    void GDTApp::ProcessResponsePacket(FANETHeader* header, Ptr<Packet> packet)
    {
        
    }

    void GDTApp::EnableInfoLog()
    {
        LogComponentEnable("GDTApp", LOG_LEVEL_INFO);
    }

    void GDTApp::EnableDebugLog()
    {
        LogComponentEnable("GDTApp", LOG_LEVEL_DEBUG);
    }
}