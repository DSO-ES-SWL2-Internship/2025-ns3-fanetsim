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

            NetDeviceContainer GDTDevice;
            std::vector<NetDeviceContainer> clustersDevices;
            std::vector<std::vector<NetDeviceContainer>> clustersLinkDevices;
            std::vector<std::vector<Ptr<NetDevice>>> linksDevices;

            FANETDeviceHelper();                                                //done
            ~FANETDeviceHelper();                                               //done

            void DefaultWifi();                                                 //done
            void TdmaWifi();                    
            void SetupGDTWifi(NodeContainer GDTNode);
            void SetupClustersWifi(std::vector<NodeContainer> clusters);        //done
            void SetUpLinksWifi(FANETTopologyHelper* fanet);
            void AssignTdmaSlots(NodeContainer nodes, Time cycleDuration);
            void AssignClusterHeads(FANETTopologyHelper* fanet, FANETAddressHelper* ipv4, FANETAnimationHelper* anim);
            void ReassignClusterHeads(FANETTopologyHelper* fanet, FANETAddressHelper* ipv4, FANETAnimationHelper* anim);
    }; 
}


#endif