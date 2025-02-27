#ifndef GDT_APP_H
#define GDT_APP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

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

            void HandleRead(Ptr<Socket> socket);
            Ptr<Socket> m_socket;
            uint16_t m_port;
    };
}


#endif