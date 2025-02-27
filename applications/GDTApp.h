#ifndef GDT_APP_H
#define GDT_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"

namespace ns3
{
    class GDTApp : public Application
    {
        public:
            GDTApp();
            ~GDTApp();

            void SetPort(uint16_t port);

        private:
            virtual void StartApplication() override;
            virtual void StopApplication() override;
            virtual void DoInitialize() override;

            void HandleRead();
            Ptr<Socket> m_socket;
            uint16_t m_port;
    };
}


#endif