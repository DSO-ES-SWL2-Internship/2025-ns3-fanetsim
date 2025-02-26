#ifndef ADD_CLIENT_H
#define ADD_CLIENT_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"


using namespace ns3;

namespace ns3
{
    class AddClient : public Application {

        private:
            Ptr<Socket> m_socket;
            Address m_peerAddress;
            uint16_t m_peerPort;
            EventId m_sendEvent;
            Ptr<UniformRandomVariable> m_randomTime;
            Ptr<UniformRandomVariable> m_randomValue;

            void SendPacket();
            void HandleRead(Ptr<Socket> socket);



        public:
            AddClient();

            void Setup(Address peerAddress, uint16_t peerPort);

            void StartApplication() override;

            void StopApplication() override;

    };
}

#endif
