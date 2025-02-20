#include "FANETDeviceHelper.h"
#include "ns3/core-module.h"
#include "ns3/FANETMobilityHelper.h"

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

    // Default settings for WiFi connections
    void FANETDeviceHelper::DefaultWifi() {
        // Configure default settings for WiFi
        clusterWifiStandard = WIFI_STANDARD_80211b;
        clusterWifiChannelPropagationDelay = "ns3::ConstantSpeedPropagationDelayModel";
        clusterPropagationLossModel = "ns3::FriisPropagationLossModel";
        clusterMacType = "ns3::AdhocWifiMac";
    }

    void FANETDeviceHelper::TdmaWifi(){
        clusterWifiStandard = WIFI_STANDARD_80211b;
        clusterWifiChannelPropagationDelay = "ns3::ConstantSpeedPropagationDelayModel";
        clusterPropagationLossModel = "ns3::FriisPropagationLossModel";
        clusterMacType = "ns3::TdmaWifiMac";
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
            
            // Edit the .h file if custom propagation delay and model is required
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
            wifiMacCM.SetType(clusterMacType, "Ssid", SsidValue(Ssid(ssid)));

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
            wifiMacAdHoc.SetType(clusterMacType, "Ssid", SsidValue(Ssid(ssid)));

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

    // Version 2 create the link for nodes to the GDT to prepare for dynamic assignment of CH during the simulation
    void FANETDeviceHelper::SetUpLinksWifiV2(FANETTopologyHelper* fanet) {
        // Ensure the WiFi standard is set
        wifi.SetStandard(clusterWifiStandard);

        for (size_t i = 0; i < fanet->clusters.size(); i++){
            std::vector<NetDeviceContainer> clusterLinks;
            clustersLinkDevices.push_back(clusterLinks);
        }

        // Clear any existing devices
        // GDTtoCHLinksDevices.clear();

        // Iterate over each cluster, and create the link between each cluster's node and the GDT
        for (size_t i = 0; i < fanet->clusters.size(); i++) {
            //NS_LOG_UNCOND("Entered 1 " << int(fanet->clusters.size()));
            for (uint32_t j = 0; j < fanet->clusters[i].GetN(); j++)
            {
                //NS_LOG_UNCOND("Entered 2 " << fanet->clusters[i].GetN());
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
                ssidStream << "Link_" << i << "_" << j;
                std::string ssid = ssidStream.str();

                // Configure the MAC layer for AdHoc (for both GDT and CH)
                WifiMacHelper wifiMacAdHoc;
                wifiMacAdHoc.SetType(clusterMacType, "Ssid", SsidValue(Ssid(ssid)));

                // Install the WiFi device on the GDT node (AdHoc mode)
                NetDeviceContainer adhocDeviceGDT = wifi.Install(wifiPhyLink, wifiMacAdHoc, fanet->GDTNode.Get(0));

                // Install the WiFi device on the CH node (AdHoc mode)
                NetDeviceContainer adhocDeviceClusterNode = wifi.Install(wifiPhyLink, wifiMacAdHoc, fanet->clusters[i].Get(j));

                // Combine the devices into a single container for this link
                NetDeviceContainer linkDevices;
                linkDevices.Add(adhocDeviceGDT);
                linkDevices.Add(adhocDeviceClusterNode);

                clustersLinkDevices[i].push_back(linkDevices);
            }
        }
    }


    void FANETDeviceHelper::AssignTdmaSlots(NodeContainer nodes, Time cycleDuration){
        for (uint32_t i = 0; i < nodes.GetN(); ++i)
        {
            Ptr<Node> node = nodes.Get(i);
            
            // Iterate through all devices of this node
            for (uint32_t j = 0; j < node->GetNDevices(); ++j)
            {
                Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(node->GetDevice(j));
                
                // Check if it's a valid WifiNetDevice
                if (wifiDevice)
                {
                    Ptr<TdmaWifiMac> tdmaMac = DynamicCast<TdmaWifiMac>(wifiDevice->GetMac());
                    
                    // Ensure that the device has a valid TDMA MAC
                    if (tdmaMac)
                    {
                        // Assign the TDMA slot for this device
                        tdmaMac->SetTdmaParameters(nodes.GetN(), cycleDuration, node->GetId());
                        
                        // Start TDMA for this device
                        tdmaMac->StartTdma();
                    }
                }
            }
        }
    }


    void FANETDeviceHelper::AssignClusterHeads(FANETTopologyHelper* fanet, FANETAddressHelper* ipv4)
    {
        for (size_t i = 0; i < fanet->clusters.size(); i++)
        {
            // Obtain the closest node of the cluster to the GDT 
            Ptr<Node> closestNode = FANETMobilityHelper::GetClosestNode(fanet->GDTNode.Get(0), fanet->clusters[i]);

            for (size_t j = 0; j < ipv4->clustersLinkInterfaces[i].size(); j++)
            {
                Ptr<Node> clusterNode = ipv4->clustersLinkInterfaces[i][j].Get(1).first->GetObject<Node>();

                if (closestNode->GetId() == clusterNode->GetId())
                {
                    Ptr<Ipv4> gdtIpv4Ptr = ipv4->clustersLinkInterfaces[i][j].Get(0).first;
                    Ptr<Ipv4> clusterNodeIpv4Ptr = ipv4->clustersLinkInterfaces[i][j].Get(1).first;
                    uint32_t gdtInterfaceIndex = ipv4->clustersLinkInterfaces[i][j].Get(0).second;
                    uint32_t clusterNodeInterfaceIndex = ipv4->clustersLinkInterfaces[i][j].Get(1).second;

                    gdtIpv4Ptr->SetUp(gdtInterfaceIndex);
                    clusterNodeIpv4Ptr->SetUp(clusterNodeInterfaceIndex);

                    // fanet->clusterHeadNodes.Add(clusterNode);
                    // NodeContainer linkNodes;
                    // linkNodes.Add(fanet->GDTNode.Get(0));
                    // linkNodes.Add(clusterNode);
                    // fanet->GDTtoCHLinkNodes.push_back(linkNodes);

                    // GDTtoCHLinksDevices.push_back(clustersLinkDevices[i][j]);

                    // ipv4->GDTtoCHLinksInterfaces.push_back(ipv4->clustersLinkInterfaces[i][j]);

                    fanet->CHNodes.push_back(clusterNode);
                    std::vector<Ptr<Node>> link;
                    link.push_back(fanet->GDTNode.Get(0));
                    link.push_back(clusterNode);
                    fanet->links.push_back(link);

                    std::vector<Ptr<NetDevice>> linkDevice;
                    linkDevice.push_back(clustersLinkDevices[i][j].Get(0));
                    linkDevice.push_back(clustersLinkDevices[i][j].Get(1));
                    linksDevices.push_back(linkDevice);
                    
                    std::vector<std::pair<Ptr<Ipv4>, uint32_t>> linkInterface;
                    linkInterface.push_back(ipv4->clustersLinkInterfaces[i][j].Get(0));
                    linkInterface.push_back(ipv4->clustersLinkInterfaces[i][j].Get(1));
                    ipv4->linksInterfaces.push_back(linkInterface);

                    NS_LOG_UNCOND("At time " << Simulator::Now().GetSeconds() << "s, Node " 
                          << clusterNode->GetId() << " selected as cluster head");

                    break;
                }
            }
        }

        Simulator::Schedule(Seconds(1.0), &FANETDeviceHelper::ReassignClusterHeads, this, fanet, ipv4);
    }

    void FANETDeviceHelper::ReassignClusterHeads(FANETTopologyHelper* fanet, FANETAddressHelper* ipv4)
    {
        for (size_t i = 0; i < fanet->clusters.size(); i++)
        {
            // Obtain the closest node of the cluster to the GDT 
            Ptr<Node> closestNode = FANETMobilityHelper::GetClosestNode(fanet->GDTNode.Get(0), fanet->clusters[i]);
            
            // Obtain the current CH
            Ptr<Node> clusterHeadNode = fanet->CHNodes[i];

            if (closestNode->GetId() != clusterHeadNode->GetId())
            {
                // Disable the previous link
                Ptr<Ipv4> curGdtIpv4Ptr = ipv4->linksInterfaces[i][0].first;
                Ptr<Ipv4> curCHIpv4Ptr = ipv4->linksInterfaces[i][1].first;
                uint32_t gdtInterfaceIndex = ipv4->linksInterfaces[i][0].second;
                uint32_t clusterNodeInterfaceIndex = ipv4->linksInterfaces[i][1].second;

                curGdtIpv4Ptr->SetDown(gdtInterfaceIndex);
                curCHIpv4Ptr->SetDown(clusterNodeInterfaceIndex);

                for (size_t j = 0; j < ipv4->clustersLinkInterfaces[i].size(); j++)
                {
                    Ptr<Node> clusterNode = ipv4->clustersLinkInterfaces[i][j].Get(1).first->GetObject<Node>();
                    // Loop through the clustersLinkInterfaces to find the appropriate index for the closest node
                    if (closestNode->GetId() == clusterNode->GetId())
                    {
                        Ptr<Ipv4> gdtIpv4Ptr = ipv4->clustersLinkInterfaces[i][j].Get(0).first;
                        Ptr<Ipv4> clusterNodeIpv4Ptr = ipv4->clustersLinkInterfaces[i][j].Get(1).first;
                        uint32_t gdtInterfaceIndex = ipv4->clustersLinkInterfaces[i][j].Get(0).second;
                        uint32_t clusterNodeInterfaceIndex = ipv4->clustersLinkInterfaces[i][j].Get(1).second;

                        gdtIpv4Ptr->SetUp(gdtInterfaceIndex);
                        clusterNodeIpv4Ptr->SetUp(clusterNodeInterfaceIndex);   

                        fanet->CHNodes[i] = closestNode;

                        std::vector<Ptr<Node>> link;
                        link.push_back(fanet->GDTNode.Get(0));
                        link.push_back(clusterNode);
                        fanet->links[i].clear();
                        fanet->links[i] = link;

                        std::vector<Ptr<NetDevice>> linkDevice;
                        linkDevice.push_back(clustersLinkDevices[i][j].Get(0));
                        linkDevice.push_back(clustersLinkDevices[i][j].Get(1));
                        linksDevices[i].clear();
                        linksDevices[i] = linkDevice;
                        
                        std::vector<std::pair<Ptr<Ipv4>, uint32_t>> linkInterface;
                        linkInterface.push_back(ipv4->clustersLinkInterfaces[i][j].Get(0));
                        linkInterface.push_back(ipv4->clustersLinkInterfaces[i][j].Get(1));
                        ipv4->linksInterfaces[i].clear();
                        ipv4->linksInterfaces[i] = linkInterface;

                        NS_LOG_UNCOND("At time " << Simulator::Now().GetSeconds() << "s, Node " 
                          << clusterNode->GetId() << " reassigned as cluster head");
                        break;
                    }
                }

            }
        }  

        Simulator::Schedule(Seconds(1.0), &FANETDeviceHelper::ReassignClusterHeads, this, fanet, ipv4);      
    }
}

