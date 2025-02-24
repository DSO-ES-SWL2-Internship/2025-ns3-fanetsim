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

            /**
             * @brief Create a cluster with nClusterMems nodes
             * 
             * @param nClusterMems Number of nodes in the cluster
             */
            NodeContainer CreateCluster(uint32_t nClusterMems);

            /**
             * @brief Storing the cluster nodes in a manner that will allow for better
             */
            void StoreClusterMembers();

            void CreateClusters(uint32_t nClusters, std::vector<uint32_t> nClusterNodes);
            void ApplyMobility();

        public:

            /// @brief Container storing all nodes in the FANET.
            NodeContainer allNodes;

            /// @brief Container storing the Ground Data Terminal (GDT) node.
            NodeContainer GDTNode;

            /// @brief Vector of NodeContainers, where each container stores all nodes within a cluster.
            std::vector<NodeContainer> clusters; 

            /// @brief Vector storing pointers to all current cluster head (CH) nodes.
            std::vector<Ptr<Node>> CHNodes;
            /// @brief Vector of vectors, where each inner vector contains pointers to nodes forming links.
            std::vector<std::vector<Ptr<Node>>> links;

            // Variables that might not be used
            std::vector<NodeContainer> clustersCMNodes;
            NodeContainer clusterHeadNodes;
            std::vector<NodeContainer> GDTtoCHLinkNodes;

            /** 
             * @brief Create a FANETTopology with nClusters, each cluster having nClusterMems
             * 
             * @param nClusters Number of clusters
             * 
             * @param nClusterMems Number of cluster nodes per cluster
             */                                  
            FANETTopologyHelper(uint32_t nClusters, uint32_t nClusterMems); 

            /**
             * @brief Create a FANETTopology with nClusters, each cluster having their specified number of nodes
             * 
             * @param nClusters Number of clusters
             * 
             * @param nClusterNodes Vector storing the number of nodes of each cluster
             */
            FANETTopologyHelper(uint32_t nClusters, std::vector<uint32_t> nClusterNodes);

            /// @brief FANETTopologyHelper Destructor
            ~FANETTopologyHelper();                                                     

            void CreateFANET(uint32_t nClusters, std::vector<uint32_t> nClusterNodes);                                                         //done
    };
}

#endif