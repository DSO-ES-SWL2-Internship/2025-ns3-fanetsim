#ifndef FANET_SIMULATOR_H
#define FANET_SIMULATOR_H

// Helper Classes Includes
#include "ns3/fanet-module.h"

namespace ns3 
{
    class FANETSimulator
    {
        private:

            // Variables to store basic information of the network

            /// @brief Number of clusters in the FANET.
            uint32_t nClusters;

            /// @brief Number of cluster nodes in each cluster.
            std::vector<uint32_t> nClusterNodes;

            /// @brief File name for NetAnim to run on.
            std::string fileName;

            /// @brief Duration of each cycle in TDMA
            uint32_t cycleDuration;

            
            /// Variables to keep track of all network interfaces in the FANET  

            /// @brief Vector storing the IPv4 interfaces of each cluster, enabling intra-cluster communication.
            std::vector<Ipv4InterfaceContainer> fanetClusterInterfaces;  

            /// @brief Vector of vectors, where each inner vector contains the IPv4 interfaces for links connecting cluster nodes to the GDT.
            std::vector<std::vector<Ipv4InterfaceContainer>> fanetClusterLinkInterfaces;  

            /// @brief Vector of vectors, where each inner vector stores a pair containing a pointer to an IPv4 instance and its corresponding interface index for each GDT-CH link.
            std::vector<std::vector<std::pair<Ptr<Ipv4>, uint32_t>>> fanetGDT_CHLinkInterfaces;  

            // Helper functions

            /// @brief Helper for managing FANET topology.
            FANETTopologyHelper* fanet;

            /// @brief Helper for configuring FANET devices.
            FANETDeviceHelper* fanetDevices;

            /// @brief Helper for handling FANET node mobility.
            FANETMobilityHelper* mobility;

            /// @brief Helper for setting up FANET routing protocols.
            FANETRoutingHelper* router;

            /// @brief Helper for managing FANET IPv4 addressing.
            FANETAddressHelper* ipv4;

            /// @brief Pointer to the FANET animation helper for visualization.
            FANETAnimationHelper *anim;


            // Methods
            
            /// @brief Obtain from user, the number of clusters to simulate
            void GetNClusters();
            
            /// @brief Obtain from user, the number of nodes in each cluster to simulator
            void GetNClusterNodes();

            /**
             * @brief Setting all the clusters to have nClusterMem nodes
             * 
             * @param nClusters Number of clusters
             * 
             * @param nClusterMem Number of nodes per cluster
             */
            void SetConstNClusterNodes(uint32_t nClusters, uint32_t nClusterNodes);

            /// @brief Obtain from user, the duration of each frame in TDMA
            void GetCycleDuration();

            void CreateNetwork();
            void InstallDevices();
            void SetMobility();
            void SetRoutingProtocol(RoutingProtocol protocol);
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

    };
}

#endif