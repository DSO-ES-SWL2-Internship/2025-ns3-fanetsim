#ifndef FANET_DEVICE_HELPER_H
#define FANET_DEVICE_HELPER_H

#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/FANETTopologyHelper.h"
#include "ns3/FANETAddressHelper.h"
#include "ns3/FANETAnimationHelper.h"


#include <cstdint>
#include <vector>
#include <string>

namespace ns3 
{
    class FANETDeviceHelper {
        private:
            std::string P2PDeviceDataRate = "";
            std::string P2PChannelDelay = "";

            WifiStandard clusterWifiStandard = WIFI_STANDARD_80211b;
            std::string clusterWifiChannelPropagationDelay = "";
            std::string clusterPropagationLossModel = "";
            std::string clusterMacType = "";

            WifiStandard linkWifiStandard = WIFI_STANDARD_80211b;
            std::string linkWifiChannelPropagationDelay = "";
            std::string linkPropagationLossModel = "";
            std::string linkMacType = "";



        public:
            PointToPointHelper p2p;
            WifiHelper wifi;
            std::vector<NetDeviceContainer> clustersDevices;
            std::vector<NetDeviceContainer> GDTtoCHLinksDevices;
            std::vector<std::vector<NetDeviceContainer>> clustersLinkDevices;

            std::vector<std::vector<Ptr<NetDevice>>> linksDevices;

            /// @brief Vector of NetDeviceContainers, where each container holds all network devices installed on the nodes of a cluster.
            std::vector<NetDeviceContainer> fanetClusterDevices;  

            /// @brief Vector of vectors, where each inner vector contains network devices used for links between nodes in a cluster and the GDT.
            std::vector<std::vector<NetDeviceContainer>> fanetClusterLinkDevices;  

            /// @brief Vector of vectors, where each inner vector contains pointers to the NetDevice of the GDT and its corresponding cluster head.
            std::vector<std::vector<Ptr<NetDevice>>> fanetGDT_CHLinkDevices;  



            FANETDeviceHelper();                                                //done
            ~FANETDeviceHelper();                                               //done

            void DefaultP2P();                                                  //done
            void DefaultWifi();                                                 //done
            void TdmaWifi();                    
            void SetupClustersWifi(std::vector<NodeContainer> clusters);        //done
            void SetupLinksP2P(std::vector<NodeContainer> GDTtoCHLinkNodes);    //done
            void SetupLinksWifi(std::vector<NodeContainer> GDTtoCHLinkNodes);   //done
            void SetUpLinksWifiV2(FANETTopologyHelper* fanet);
            void AssignTdmaSlots(NodeContainer nodes, Time cycleDuration);
            void AssignClusterHeads(FANETTopologyHelper* fanet, FANETAddressHelper* ipv4, FANETAnimationHelper* anim);
            void ReassignClusterHeads(FANETTopologyHelper* fanet, FANETAddressHelper* ipv4, FANETAnimationHelper* anim);
    }; 
}


#endif