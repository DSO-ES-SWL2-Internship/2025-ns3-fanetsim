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

        //NS_LOG_DEBUG("Node " << GetNode()->GetId() << " application to notify gdt that it became clusterhead started");
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

            RegisterHandlers();
        }

        Application::DoInitialize();
    }



    void ClusterNodeApp::HandleRead(Ptr<Socket> socket)
    {
        FANETCommunication::ReceivePacket(this, socket);
    }


    void ClusterNodeApp::RegisterHandlers() 
    {
        FANETApplication::RegisterHandlers();

        helloServiceHandlers[CH_PROMO] = [this] (FANETHeader* header, Ptr<Packet> packet, Address from ) { HandleCHPromo(header, packet, from); };
    }

    void ClusterNodeApp::EnableInfoLog()
    {
        FANETApplication::EnableInfoLog();
        LogComponentEnable("ClusterNodeApp", LOG_LEVEL_INFO);
        LogComponentEnable("ClusterNodeCHPromo", LOG_LEVEL_INFO);
    }

    void ClusterNodeApp::EnableDebugLog()
    {
        FANETApplication::EnableDebugLog();
        LogComponentEnable("ClusterNodeApp", LOG_LEVEL_DEBUG);
        LogComponentEnable("ClusterNodeCHPromo", LOG_LEVEL_DEBUG);
    }
}