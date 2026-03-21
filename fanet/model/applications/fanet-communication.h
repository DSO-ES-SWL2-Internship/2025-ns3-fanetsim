#ifndef FANET_COMMUNICATION_H
#define FANET_COMMUNICATION_H

#include "ns3/fanet-application.h"

namespace ns3 
{
    class FANETCommunication
    {
        public:
            static int SendPacket(Ptr<FANETApplication> app, Ptr<Packet> packet, Ipv4Address destAddr);
            static void ReceivePacket(Ptr<FANETApplication> app, Ptr<Socket> socket);
    };
}

#endif