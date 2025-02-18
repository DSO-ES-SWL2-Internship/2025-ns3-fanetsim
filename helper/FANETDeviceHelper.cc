#include "FANETDeviceHelper.h"
#include "ns3/core-module.h"

namespace ns3
{
        // Constructor
    FANETDeviceHelper::FANETDeviceHelper() {
        // Constructor initialization logic (if any)
    }

    // Destructor
    FANETDeviceHelper::~FANETDeviceHelper() {
        // Cleanup logic (if necessary)
    }

    // Default settings for Point-to-Point connections
    void FANETDeviceHelper::DefaultP2P() {
        // Configure default settings for P2P links
        P2PDeviceDataRate = "5Mbps";
        P2PChannelDelay = "2ms";

        clusterWifiStandard = WIFI_STANDARD_80211b;
        clusterWifiChannelPropagationDelay = "ns3::ConstantSpeedPropagationDelayModel";
        clusterPropagationLossModel = "ns3::FriisPropagationLossModel";
        clusterMacType = "ns3::AdhocWifiMac";
    }

    // TODO
    // Default settings for WiFi connections
    void FANETDeviceHelper::DefaultWifi() {
        // Configure default settings for WiFi
    }

    void FANETDeviceHelper::SetupClustersWifi(std::vector<NodeContainer> clusters) {
        // Ensure the WiFi standard is set
        wifi.SetStandard(clusterWifiStandard);

        // Clear any existing devices
        //clustersDevices.clear();

        // Install WiFi devices for each cluster
        for (size_t i = 0; i < clusters.size(); i++) {
            
            // Create a unique Wi-Fi channel for this cluster
            YansWifiChannelHelper wifiChannelCM;
            
            // Configure the channel properties (if specified)
            if (!clusterWifiChannelPropagationDelay.empty()) {
                wifiChannelCM.SetPropagationDelay(clusterWifiChannelPropagationDelay);
            }
            if (!clusterPropagationLossModel.empty()) {
                wifiChannelCM.AddPropagationLoss(clusterPropagationLossModel);
            }

            // Setup the PHY layer for this cluster with the unique channel
            YansWifiPhyHelper wifiPhyCluster;
            wifiPhyCluster.SetChannel(wifiChannelCM.Create());

            // Configure SSID for the cluster
            std::ostringstream ssidStream;
            ssidStream << "Cluster_" << i;
            std::string ssid = ssidStream.str();

            // Configure the MAC layer for AdHoc (for both GDT and CH)
            WifiMacHelper wifiMacCM;
            wifiMacCM.SetType("ns3::AdhocWifiMac", "Ssid", SsidValue(Ssid(ssid)));

            // Install WiFi devices on nodes in the current cluster
            NetDeviceContainer clusterDevices = wifi.Install(wifiPhyCluster, wifiMacCM, clusters[i]);
            clustersDevices.push_back(clusterDevices);
        }
    }



    // Setup P2P links
    void FANETDeviceHelper::SetupLinksP2P(std::vector<NodeContainer> GDTtoCHLinkNodes) {

        // ==================== Setup Stage ===============================
        p2p.SetDeviceAttribute("DataRate", StringValue(P2PDeviceDataRate));
        p2p.SetChannelAttribute("Delay", StringValue(P2PChannelDelay));

        // ==================== Install Stage ===================================
        for (size_t i = 0; i < GDTtoCHLinkNodes.size(); i++){
            GDTtoCHLinksDevices.push_back(p2p.Install(GDTtoCHLinkNodes[i]));
            //NS_LOG_UNCOND("Link size: " << GDTtoCHLinksDevices[i].GetN());
        }
    }

    // TODO
    // void FANETDeviceHelper::SetupLinksWifi(std::vector<NodeContainer> GDTtoCHLinkNodes) {
    //     // Ensure the WiFi standard is set
    //     wifi.SetStandard(clusterWifiStandard);

    //     // Create a separate WiFi channel for CH-GDT links
    //     YansWifiChannelHelper wifiChannelGDT;
    //     if (!clusterWifiChannelPropagationDelay.empty()) {
    //         wifiChannelGDT.SetPropagationDelay(clusterWifiChannelPropagationDelay);
    //     }
    //     if (!clusterPropagationLossModel.empty()) {
    //         wifiChannelGDT.AddPropagationLoss(clusterPropagationLossModel);
    //     }

    //     // Setup the PHY layer for CH-GDT links
    //     wifiPhyGDT.SetChannel(wifiChannelGDT.Create());

    //     // Clear any existing devices
    //     //GDTtoCHLinksDevices.clear();

    //     // Iterate over each CH-GDT pair and create a separate WiFi network for each
    //     for (size_t i = 0; i < GDTtoCHLinkNodes.size(); i++) {
    //         // Create a unique SSID for each CH-GDT pair
    //         std::ostringstream ssidStream;
    //         ssidStream << "CH_GDT_Link_" << i;
    //         std::string ssid = ssidStream.str();

    //         // Configure the MAC layer for the GDT (Access Point)
    //         WifiMacHelper wifiMacGDT;
    //         wifiMacGDT.SetType("ns3::ApWifiMac", "Ssid", SsidValue(Ssid(ssid)));

    //         // Install the WiFi device on the GDT node
    //         NetDeviceContainer apDevice = wifi.Install(wifiPhyGDT, wifiMacGDT, GDTtoCHLinkNodes[i].Get(0));

    //         // Configure the MAC layer for the CH (Station)
    //         wifiMacGDT.SetType("ns3::StaWifiMac", "Ssid", SsidValue(Ssid(ssid)), "ActiveProbing", BooleanValue(false));

    //         // Install the WiFi device on the CH node
    //         NetDeviceContainer staDevice = wifi.Install(wifiPhyGDT, wifiMacGDT, GDTtoCHLinkNodes[i].Get(1));

    //         // Combine the devices into a single container for this link
    //         NetDeviceContainer linkDevices;
    //         linkDevices.Add(apDevice);
    //         linkDevices.Add(staDevice);

    //         // Add the link devices to the global container
    //         GDTtoCHLinksDevices.push_back(linkDevices);
    //     }
    // }

    void FANETDeviceHelper::SetupLinksWifi(std::vector<NodeContainer> GDTtoCHLinkNodes) {
        // Ensure the WiFi standard is set
        wifi.SetStandard(clusterWifiStandard);

        // Clear any existing devices
        // GDTtoCHLinksDevices.clear();

        // Iterate over each CH-GDT pair and create a separate WiFi network for each
        for (size_t i = 0; i < GDTtoCHLinkNodes.size(); i++) {
                    // Create a separate WiFi channel for CH-GDT links
            YansWifiChannelHelper wifiChannelLink;
            if (!clusterWifiChannelPropagationDelay.empty()) {
                wifiChannelLink.SetPropagationDelay(clusterWifiChannelPropagationDelay);
            }
            if (!clusterPropagationLossModel.empty()) {
                wifiChannelLink.AddPropagationLoss(clusterPropagationLossModel);
            }

            // Setup the PHY layer for CH-GDT links
            YansWifiPhyHelper wifiPhyLink;
            wifiPhyLink.SetChannel(wifiChannelLink.Create());
            // Create a unique SSID for each CH-GDT pair
            std::ostringstream ssidStream;
            ssidStream << "CH_GDT_Link_" << i;
            std::string ssid = ssidStream.str();

            // Configure the MAC layer for AdHoc (for both GDT and CH)
            WifiMacHelper wifiMacAdHoc;
            wifiMacAdHoc.SetType("ns3::AdhocWifiMac", "Ssid", SsidValue(Ssid(ssid)));

            // Install the WiFi device on the GDT node (AdHoc mode)
            NetDeviceContainer adhocDeviceGDT = wifi.Install(wifiPhyLink, wifiMacAdHoc, GDTtoCHLinkNodes[i].Get(0));

            // Install the WiFi device on the CH node (AdHoc mode)
            NetDeviceContainer adhocDeviceCH = wifi.Install(wifiPhyLink, wifiMacAdHoc, GDTtoCHLinkNodes[i].Get(1));

            // Combine the devices into a single container for this link
            NetDeviceContainer linkDevices;
            linkDevices.Add(adhocDeviceGDT);
            linkDevices.Add(adhocDeviceCH);

            // Add the link devices to the global container
            GDTtoCHLinksDevices.push_back(linkDevices);
        }
    }


}