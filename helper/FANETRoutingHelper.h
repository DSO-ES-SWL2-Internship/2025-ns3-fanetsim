#ifndef FANET_ROUTING_HELPER_H
#define FANET_ROUTING_HELPER_H

#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"

namespace ns3
{
    enum RoutingProtocol {
        AODV,
        OLSR,
        DSDV,
        DSR,
    };

    class FANETRoutingHelper {
        private:
            InternetStackHelper internet;
            Ipv4ListRoutingHelper list;

            void InstallInternetStackToAllNodes(NodeContainer nodes);

        public:
            FANETRoutingHelper();
            ~FANETRoutingHelper();

            // Methods to set the routing protocol used
            void SetAODV(NodeContainer nodes);
            void SetOLSR(NodeContainer nodes);
            void SetDSDV(NodeContainer nodes);
            //void SetDSR(NodeContainer nodes);
    };
}

#endif