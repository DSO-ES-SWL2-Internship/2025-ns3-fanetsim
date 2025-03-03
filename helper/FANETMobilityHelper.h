#ifndef FANET_MOBILITY_HELPER_H
#define FANET_MOBILITY_HELPER_H

#include "ns3/mobility-module.h"
#include "FANETTopologyHelper.h"


#include <cstdint>

namespace ns3
{
    class FANETMobilityHelper {
        public:
            FANETMobilityHelper();                                                                                                      
            ~FANETMobilityHelper();                                                                                                     

            void SetGDTMobility(NodeContainer& gdtNode);                                                                                                                                                          
            void ApplyMobilityWireless(FANETTopologyHelper* fanet);
            static Ptr<Node> GetClosestNode(Ptr<Node> target, NodeContainer nodes);

            // void SetClusterHeadMobility(NodeContainer& clusterHeads, double x, double y, uint32_t nClusterHeads, double radius);        
            // void SetClusterMemberMobility(NodeContainer& clusterMembers, double xCenter, double yCenter);                               
            // void ApplyMobilityP2P(FANETTopologyHelper* fanet);   

        private:
            MobilityHelper mobility;
    };
}

#endif