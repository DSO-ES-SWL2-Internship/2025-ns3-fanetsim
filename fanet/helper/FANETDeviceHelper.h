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
    /**
     * @class FANETDeviceHelper
     * @brief Helper class for setting up FANET network devices.
     */
    class FANETDeviceHelper : public Object {
    private:
        WifiStandard clusterWifiStandard = WIFI_STANDARD_80211b; /**< WiFi standard used for clusters. */
        std::string clusterWifiChannelPropagationDelay; /**< Propagation delay model for cluster WiFi channels. */
        std::string clusterPropagationLossModel; /**< Propagation loss model for clusters. */
        std::string clusterMacType; /**< MAC type used for cluster devices. */

        WifiStandard linkWifiStandard = WIFI_STANDARD_80211b; /**< WiFi standard used for links. */
        std::string linkWifiChannelPropagationDelay; /**< Propagation delay model for link WiFi channels. */
        std::string linkPropagationLossModel; /**< Propagation loss model for links. */
        std::string linkMacType; /**< MAC type used for link devices. */

    public:
        WifiHelper wifi; /**< WiFi helper for device configuration. */

        NetDeviceContainer GDTDevice; /**< Container for GDT network devices. */
        std::vector<NetDeviceContainer> clustersDevices; /**< Containers for cluster devices. */
        std::vector<std::vector<NetDeviceContainer>> clustersLinkDevices; /**< Containers for cluster link devices. */
        std::vector<std::vector<Ptr<NetDevice>>> linksDevices; /**< Pointers to link devices. */

        static TypeId GetTypeId();

        /**
         * @brief Constructor for FANETDeviceHelper.
         */
        FANETDeviceHelper();

        /**
         * @brief Destructor for FANETDeviceHelper.
         */
        ~FANETDeviceHelper();

        /**
         * @brief Configures default WiFi settings.
         */
        void DefaultWifi();

        /**
         * @brief Configures TDMA-based WiFi settings.
         */
        void TdmaWifi();

        /**
         * @brief Sets up WiFi for the GDT node.
         * @param GDTNode The node container for GDT.
         */
        void SetupGDTWifi(NodeContainer GDTNode);

        /**
         * @brief Sets up WiFi for cluster nodes.
         * @param clusters A vector containing cluster node containers.
         */
        void SetupClustersWifi(std::vector<NodeContainer> clusters);

        /**
         * @brief Sets up WiFi links between clusters.
         * @param fanet A pointer to the FANETTopologyHelper instance.
         */
        void SetUpLinksWifi(Ptr<FANETTopologyHelper> fanet);

        /**
         * @brief Assigns TDMA slots to nodes.
         * @param nodes The node container.
         * @param cycleDuration The duration of a TDMA cycle.
         */
        void AssignTdmaSlots(NodeContainer nodes, Time cycleDuration);

        /**
         * @brief Assigns cluster heads in the network.
         * @param fanet Pointer to the FANET topology helper.
         * @param ipv4 Pointer to the FANET address helper.
         * @param anim Pointer to the FANET animation helper.
         */
        void AssignClusterHeads(Ptr<FANETTopologyHelper> fanet, Ptr<FANETAddressHelper> ipv4, FANETAnimationHelper* anim);

        /**
         * @brief Reassigns cluster heads dynamically.
         * @param fanet Pointer to the FANET topology helper.
         * @param ipv4 Pointer to the FANET address helper.
         * @param anim Pointer to the FANET animation helper.
         */
        void ReassignClusterHeads(Ptr<FANETTopologyHelper> fanet, Ptr<FANETAddressHelper> ipv4, FANETAnimationHelper* anim);

        /**
         * @brief Notifies a change in cluster head status.
         * @param node Pointer to the node whose status changed.
         * @param status New status of the node.
         */
        void NotifyCHStatusChange(Ptr<Node> node, std::string status);
    }; 
}


#endif