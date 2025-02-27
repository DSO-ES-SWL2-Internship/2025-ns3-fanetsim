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

            void SetUp(Ipv4Address gdtIp, uint16_t port, uint32_t clusterIndex);
            void NotifyGDT(bool isCH);


        private:
            
            Ptr<Socket> m_socket;
            Ipv4Address m_gdtIp;
            uint16_t m_port;
            bool m_isClusterHead;
            bool m_isActive;
            EventId m_sendEvent; 
            uint32_t m_clusterIndex;
            
            virtual void StartApplication() override;
            virtual void StopApplication() override;
            virtual void DoInitialize() override;


            void HandleRead(Ptr<Socket> socket);
            void SendMessage();
            void EnableAsciiTracing(Ptr<OutputStreamWrapper> stream);
    };
}

#endif