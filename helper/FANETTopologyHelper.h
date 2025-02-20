#ifndef FANET_TOPOLOGY_HELPER_H
#define FANET_TOPOLOGY_HELPER_H

#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <cstdint>
#include <vector>

namespace ns3 
{
    class FANETTopologyHelper
    {
        private:
            
            void CreateClusters();
            NodeContainer CreateCluster();
            void StoreClusterMembers();
            void CreateP2PLinks();
            void CreateWirelessLinksv1();
            void CreateWirelessLinksv2();
            void CreateWirelessLinksv3();

        public:
            uint32_t nClusterHeads;
            uint32_t nClusterMems;
            NodeContainer allNodes;
            NodeContainer GDTNode;
            NodeContainer clusterHeadNodes;
            std::vector<NodeContainer> GDTtoCHLinkNodes;
            std::vector<NodeContainer> clusters; 
            std::vector<NodeContainer> clustersCMNodes;

            std::vector<Ptr<Node>> CHNodes;
            std::vector<std::vector<Ptr<Node>>> links;

            FANETTopologyHelper();                                                      //done
            FANETTopologyHelper(uint32_t nClusterHeads, uint32_t nClusterMems);         //done
            ~FANETTopologyHelper();                                                     //done

            void SetupFANET(uint32_t nClusterHeads, uint32_t nClusterMembers);          //done
            void CreateFANETP2P();
            void CreateFANETWirelessv1();                                                         //done
            void CreateFANETWirelessv2();
            void CreateFANETWirelessv3();
    };
}

#endif