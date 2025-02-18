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
            void CreateLinks();

        public:
            uint32_t nClusterHeads;
            uint32_t nClusterMems;
            NodeContainer allNodes;
            NodeContainer GDTNode;
            NodeContainer clusterHeadNodes;
            std::vector<NodeContainer> GDTtoCHLinkNodes;
            std::vector<NodeContainer> clusters; 
            std::vector<NodeContainer> clustersCMNodes;

            FANETTopologyHelper();                                                      //done
            FANETTopologyHelper(uint32_t nClusterHeads, uint32_t nClusterMems);         //done
            ~FANETTopologyHelper();                                                     //done

            void SetupFANET(uint32_t nClusterHeads, uint32_t nClusterMembers);          //done
            void CreateFANET();                                                         //done
    };
}

#endif