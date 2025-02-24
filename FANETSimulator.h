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

            // Number of clusters in the FANET
            uint32_t nClusters;
            // Number of cluster nodes in each cluster
            std::vector<uint32_t> nClusterNodes;
            // File name for NetAnim to run on
            std::string fileName;

            // Variables to keep track of all the nodes in the FANET and their roles

            // Container storing all nodes in the FANET
            NodeContainer allNodes;  
            // Container storing the Ground Data Terminal (GDT) node
            NodeContainer GDTNode;  
            // Vector of NodeContainers, where each container stores all nodes within a cluster  
            std::vector<NodeContainer> clusters;  
            // Vector storing pointers to all current cluster head (CH) nodes  
            std::vector<Ptr<Node>> CHNodes;  
            // Vector of vectors, where each inner vector contains pointers to nodes forming links  
            std::vector<std::vector<Ptr<Node>>> links;  

            // Variables to keep track of all network devices in the FANET  

            // Vector of NetDeviceContainers, where each container holds all network devices installed on the nodes of a cluster  
            std::vector<NetDeviceContainer> fanetClusterDevices;  
            // Vector of vectors, where each inner vector contains network devices used for links between nodes in a cluster and the GDT  
            std::vector<std::vector<NetDeviceContainer>> fanetClusterLinkDevices;  
            // Vector of vectors, where each inner vector contains pointers to the NetDevice of the GDT and its corresponding cluster head  
            std::vector<std::vector<Ptr<NetDevice>>> fanetGDT_CHLinkDevices;  

            // Variables to keep track of all network interfaces in the FANET  

            // Vector storing the IPv4 interfaces of each cluster, enabling intra-cluster communication  
            std::vector<Ipv4InterfaceContainer> fanetClusterInterfaces;  
            // Vector of vectors, where each inner vector contains the IPv4 interfaces for links connecting cluster nodes to the GDT  
            std::vector<std::vector<Ipv4InterfaceContainer>> fanetClusterLinkInterfaces;  
            // Vector of vectors, where each inner vector stores a pair containing a pointer to an IPv4 instance and its corresponding interface index for each GDT-CH link  
            std::vector<std::vector<std::pair<Ptr<Ipv4>, uint32_t>>> fanetGDT_CHLinkInterfaces;  

            // Helpers
            FANETTopologyHelper fanet;
            FANETDeviceHelper fanetDevices;
            FANETMobilityHelper mobility;
            FANETRoutingHelper router;
            FANETAddressHelper ipv4;
            FANETAnimationHelper *anim;

            // Methods
            void CreateNetwork();
            void InstallDevices();
            void SetMobility();
            void SetRoutingProtocol();
            void AssignAddress();

        public:

            FANETSimulator();
            ~FANETSimulator();

            void RunBasicSimulation(std::string fileName = "animation.xml");
            void RunBasicSimulation(uint32_t nClusters, uint32_t nClusterNodes, std::string fileName = "animation.xml");
    };
}

#endif