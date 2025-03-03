#ifndef ADD_SERVER_H
#define ADD_SERVER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"
#include <queue>

namespace ns3
{
    class AddServer : public Application {
        private:
            Ptr<Socket> m_socket;
            std::queue<Ptr<Packet>> m_packetQueue;
            std::queue<Address> m_addressQueue;

            void HandleRead(Ptr<Socket> socket);

            void ProcessNextPacket();


        public:
            AddServer() {}

            void StartApplication() override;

            void StopApplication() override;

            void EnableInfoLog();

            void EnableDebugLog();
    };
}

#endif