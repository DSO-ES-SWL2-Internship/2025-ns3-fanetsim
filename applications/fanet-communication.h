#ifndef FANET_COMMUNICATION_H
#define FANET_COMMUNICATION_H

#include "ns3/fanet-application.h"

namespace ns3 
{
    class FANETCommunication
    {
        static int SendPacket(FANETApplication* app, const Ptr<Packet> packet);
        void ReceivePacket(FANETApplication* app);
    };
}

#endif