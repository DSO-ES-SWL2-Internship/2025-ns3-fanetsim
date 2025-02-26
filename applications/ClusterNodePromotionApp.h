#ifndef CLUSTER_NODE_PROMOTION_APP_H
#define CLUSTER_NODE_PROMOTION_APP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

namespace ns3
{
    class ClusterNodePromotionApp : public Application
    {
        public:

            ClusterNodePromotionApp();
            virtual ~ClusterNodePromotionApp();

            void SetUp(Ptr<Node> GDTNode, Ipv4Address GDTIp, uint16_t port);

        private:
            
            Ptr<Socket> m_socket;
            Ipv4Address m_GDTIp;
            uint16_t m_port;
            bool isClusterHead;
            EventId m_sendEvent; 
            
            virtual void StartApplication() override;
            virtual void StopApplication() override;

            void HandleRead(Ptr<Socket> socket);
            void NotifyGDT();
    };
}

#endif