#ifndef FANET_SIMULATOR_H
#define FANET_SIMULATOR_H

// Helper Classes Includes
#include "ns3/fanet-module.h"

namespace ns3 
{
    class FANETSimulator
    {
        private:

            /// Variables to store basic information of the network

            /// @brief Number of clusters in the FANET.
            uint32_t nClusters;

            /// @brief Number of cluster nodes in each cluster.
            std::vector<uint32_t> nClusterNodes;

            /// @brief File name for NetAnim to run on.
            std::string fileName;

            /// Variables to keep track of all the nodes in the FANET and their roles

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

            /// Variables to keep track of all network devices in the FANET  

            /// @brief Vector of NetDeviceContainers, where each container holds all network devices installed on the nodes of a cluster.
            std::vector<NetDeviceContainer> fanetClusterDevices;  

            /// @brief Vector of vectors, where each inner vector contains network devices used for links between nodes in a cluster and the GDT.
            std::vector<std::vector<NetDeviceContainer>> fanetClusterLinkDevices;  

            /// @brief Vector of vectors, where each inner vector contains pointers to the NetDevice of the GDT and its corresponding cluster head.
            std::vector<std::vector<Ptr<NetDevice>>> fanetGDT_CHLinkDevices;  

            /// Variables to keep track of all network interfaces in the FANET  

            /// @brief Vector storing the IPv4 interfaces of each cluster, enabling intra-cluster communication.
            std::vector<Ipv4InterfaceContainer> fanetClusterInterfaces;  

            /// @brief Vector of vectors, where each inner vector contains the IPv4 interfaces for links connecting cluster nodes to the GDT.
            std::vector<std::vector<Ipv4InterfaceContainer>> fanetClusterLinkInterfaces;  

            /// @brief Vector of vectors, where each inner vector stores a pair containing a pointer to an IPv4 instance and its corresponding interface index for each GDT-CH link.
            std::vector<std::vector<std::pair<Ptr<Ipv4>, uint32_t>>> fanetGDT_CHLinkInterfaces;  

            /// Helper functions

            /// @brief Helper for managing FANET topology.
            FANETTopologyHelper fanet;

            /// @brief Helper for configuring FANET devices.
            FANETDeviceHelper fanetDevices;

            /// @brief Helper for handling FANET node mobility.
            FANETMobilityHelper mobility;

            /// @brief Helper for setting up FANET routing protocols.
            FANETRoutingHelper router;

            /// @brief Helper for managing FANET IPv4 addressing.
            FANETAddressHelper ipv4;

            /// @brief Pointer to the FANET animation helper for visualization.
            FANETAnimationHelper *anim;


            // Methods
            
            
            /// @brief Obtain from user, the number of clusters to simulate
            void GetNClusters();
            
            /// @brief Obtain from user, the number of nodes in each cluster to simulator
            void GetNClusterNodes();

            void CreateNetwork();
            void InstallDevices();
            void SetMobility();
            void SetRoutingProtocol();
            void AssignAddress();

        public:

            /// @brief Create a FANET Simulator
            FANETSimulator();
            /// @brief Destroy the FANET Simulator
            ~FANETSimulator();

            /**
             * @brief Run basic simulation of the fanet
             * 
             * @param fileName Name of output file neccessary for NetAnim
             */
            void RunBasicSimulation(std::string fileName = "animation.xml");

            /**
             * @brief Run basic simulation of the fanet with need to ask user information
             * 
             * @param nClusters The number of clusters to simulate
             * 
             * @param nClusterNodes The number of nodes each cluster will have
             * 
             * @param fileName Name of output file neccessary for NetAnim
             */
            void RunBasicSimulation(uint32_t nClusters, uint32_t nClusterNodes, std::string fileName = "animation.xml");
    };
}

#endif