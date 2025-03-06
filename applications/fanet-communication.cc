#include "fanet-communication.h"
#include "ns3/core-module.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("FANETCommunication");

    int FANETCommunication::SendPacket(FANETApplication* app, const Ptr<Packet> packet)
    {
        if (!app->GetSocket())
        {
            NS_LOG_ERROR("Socket is NULL");
            return -1;
        }

        Ptr<Socket> socket = app->GetSocket();
        socket->Connect(InetSocketAddress(app->GetDestAddr(), app->GetPort()));

        int sentBytes = socket->Send(packet);

        if (sentBytes > 0)
            NS_LOG_DEBUG("Packet Sent");
        else
            NS_LOG_DEBUG("Failed to send packet");

        return sentBytes;
    }

    void FANETCommunication::ReceivePacket(FANETApplication* app, Ptr<Socket> socket)
    {
        std::queue<Ptr<Packet>>& packetQueue = app->GetPacketQueue();
        std::queue<Address>& addressQueue = app->GetAddressQueue();

        Ptr<Packet> packet;
        Address from;

        while ((packet = socket->RecvFrom(from)))
        {
            NS_LOG_DEBUG("At time " << Simulator::Now().GetSeconds() << " Node " << app->GetNode()->GetId() << " received a packet of size " << packet->GetSize());
            packetQueue.push(packet);
            addressQueue.push(from);
        }

        if (!packetQueue.empty())
            Simulator::ScheduleNow(&FANETApplication::ProcessNextPacket, app);
    }
}