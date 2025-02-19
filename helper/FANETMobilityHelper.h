#ifndef FANET_MOBILITY_HELPER_H
#define FANET_MOBILITY_HELPER_H

#include "ns3/mobility-module.h"
#include "FANETTopologyHelper.h"


#include <cstdint>

namespace ns3
{
    class FANETMobilityHelper {
        public:
            FANETMobilityHelper();                                                                                                      //done
            ~FANETMobilityHelper();                                                                                                     //done

            void SetGDTMobility(NodeContainer& gdtNode);                                                                                //done
            void SetClusterHeadMobility(NodeContainer& clusterHeads, double x, double y, uint32_t nClusterHeads, double radius);        //done
            void SetClusterMemberMobility(NodeContainer& clusterMembers, double xCenter, double yCenter);                               //done
            void ApplyMobilityP2P(FANETTopologyHelper* fanet);                                                                             //done
            void ApplyMobilityWireless(FANETTopologyHelper* fanet);

        private:
            MobilityHelper mobility;
    };
}

#endif