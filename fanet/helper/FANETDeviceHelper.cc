#include "FANETDeviceHelper.h"
#include "ns3/core-module.h"
#include "ns3/FANETMobilityHelper.h"
#include "ns3/FANETHeader.h"
#include "ns3/tdma-wifi-mac.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("FANETDeviceHelper");

    TypeId FANETDeviceHelper::GetTypeId()
    {
        static TypeId tid = 
            TypeId("ns3::FANETDeviceHelper")
                .SetParent<ns3::Object>()
                .AddConstructor<FANETDeviceHelper>()
                .AddAttribute(  "clusterWifiStandard",
                                "Wifi standard to use for clusters",
                                EnumValue(WIFI_STANDARD_80211b),
                                MakeEnumAccessor<WifiStandard>(&FANETDeviceHelper::clusterWifiStandard),
                                MakeEnumChecker(
                                    WIFI_STANDARD_UNSPECIFIED, "WIFI_STANDARD_UNSPECIFIED",
                                    WIFI_STANDARD_80211a, "WIFI_STANDARD_80211A",
                                    WIFI_STANDARD_80211b, "WIFI_STANDARD_80211B",
                                    WIFI_STANDARD_80211g, "WIFI_STANDARD_80211G",
                                    WIFI_STANDARD_80211p, "WIFI_STANDARD_80211P",
                                    WIFI_STANDARD_80211n, "WIFI_STANDARD_80211N",
                                    WIFI_STANDARD_80211ac, "WIFI_STANDARD_80211AC",
                                    WIFI_STANDARD_80211ad, "WIFI_STANDARD_80211AD",
                                    WIFI_STANDARD_80211ax, "WIFI_STANDARD_80211AX",
                                    WIFI_STANDARD_80211be, "WIFI_STANDARD_80211BE"
                                ))
                .AddAttribute(  "clusterWifiChannelPropagationDelay",
                                "Propagation delay model of cluster wifi",
                                StringValue("ns3::ConstantSpeedPropagationDelayModel"),
                                MakeStringAccessor(&FANETDeviceHelper::clusterWifiChannelPropagationDelay),
                                MakeStringChecker()
                                )
                .AddAttribute(  "clusterPropagationLossModel",
                                "Propagation loss model of cluster wifi",
                                StringValue("ns3::FriisPropagationLossModel"),
                                MakeStringAccessor(&FANETDeviceHelper::clusterPropagationLossModel),
                                MakeStringChecker()
                                )
                .AddAttribute(  "clusterMacType",
                                "MAC type of the cluster",
                                StringValue("ns3::TdmaWifiMac"),
                                MakeStringAccessor(&FANETDeviceHelper::clusterMacType),
                                MakeStringChecker()
                                )      
                .AddAttribute(  "linkWifiStandard",
                                "Wifi standard to use for links",
                                EnumValue(WIFI_STANDARD_80211b),
                                MakeEnumAccessor<WifiStandard>(&FANETDeviceHelper::linkWifiStandard),
                                MakeEnumChecker(
                                    WIFI_STANDARD_UNSPECIFIED, "WIFI_STANDARD_UNSPECIFIED",
                                    WIFI_STANDARD_80211a, "WIFI_STANDARD_80211A",
                                    WIFI_STANDARD_80211b, "WIFI_STANDARD_80211B",
                                    WIFI_STANDARD_80211g, "WIFI_STANDARD_80211G",
                                    WIFI_STANDARD_80211p, "WIFI_STANDARD_80211P",
                                    WIFI_STANDARD_80211n, "WIFI_STANDARD_80211N",
                                    WIFI_STANDARD_80211ac, "WIFI_STANDARD_80211AC",
                                    WIFI_STANDARD_80211ad, "WIFI_STANDARD_80211AD",
                                    WIFI_STANDARD_80211ax, "WIFI_STANDARD_80211AX",
                                    WIFI_STANDARD_80211be, "WIFI_STANDARD_80211BE"
                                ))
                .AddAttribute(  "linkWifiChannelPropagationDelay",
                                "Propagation delay model of link wifi",
                                StringValue("ns3::ConstantSpeedPropagationDelayModel"),
                                MakeStringAccessor(&FANETDeviceHelper::linkWifiChannelPropagationDelay),
                                MakeStringChecker()
                                )
                .AddAttribute(  "linkPropagationLossModel",
                                "Propagation loss model of link wifi",
                                StringValue("ns3::FriisPropagationLossModel"),
                                MakeStringAccessor(&FANETDeviceHelper::linkPropagationLossModel),
                                MakeStringChecker()
                                )
                .AddAttribute(  "linkMacType",
                                "MAC type of the link",
                                StringValue("ns3::TdmaWifiMac"),
                                MakeStringAccessor(&FANETDeviceHelper::linkMacType),
                                MakeStringChecker()
                                );               
        return tid;
    }

    // Constructor
    FANETDeviceHelper::FANETDeviceHelper() {
    }

    // Destructor
    FANETDeviceHelper::~FANETDeviceHelper() {

    }

    void FANETDeviceHelper::DefaultWifi() {
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

    void FANETDeviceHelper::SetupGDTWifi(NodeContainer GDTNode)
    {
        WifiHelper localWifiGDT;
        localWifiGDT.SetStandard(clusterWifiStandard);
        
        // Create a separate WiFi channel for the dedicated GDT WiFi device
        YansWifiChannelHelper wifiChannelGDT;
        if (!clusterWifiChannelPropagationDelay.empty()) {
            wifiChannelGDT.SetPropagationDelay(clusterWifiChannelPropagationDelay);
        }
        if (!clusterPropagationLossModel.empty()) {
            wifiChannelGDT.AddPropagationLoss(clusterPropagationLossModel);
        }

        // Setup the PHY layer for the GDT WiFi device
        YansWifiPhyHelper wifiPhyGDT;
        wifiPhyGDT.SetChannel(wifiChannelGDT.Create());

        // Configure the MAC layer for AdHoc mode
        WifiMacHelper wifiMacAdHocGDT;
        wifiMacAdHocGDT.SetType(clusterMacType, "Ssid", SsidValue(Ssid("GDT-WiFi")));

        // Install the WiFi device on the GDT node
        NetDeviceContainer gdtWiFiDevice = localWifiGDT.Install(wifiPhyGDT, wifiMacAdHocGDT, GDTNode.Get(0));

        // Store the new device
        this->GDTDevice = gdtWiFiDevice;
    }

    //Intra-cluster f_n
    void FANETDeviceHelper::SetupClustersWifi(std::vector<NodeContainer> clusters) {
        
        WifiHelper localWifiIntra;
        // Ensure the WiFi standard is set
        localWifiIntra.SetStandard(clusterWifiStandard);
        //wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("DsssRate11Mbps"), "ControlMode", StringValue("DsssRate11Mbps"));

        // Install WiFi devices for each cluster
        for (size_t i = 0; i < clusters.size(); i++) {
            
            // Create a unique Wi-Fi channel for this cluster
            YansWifiChannelHelper wifiChannelintra;
            
            if (!clusterWifiChannelPropagationDelay.empty()) {
                wifiChannelintra.SetPropagationDelay(clusterWifiChannelPropagationDelay);
            }
            if (!clusterPropagationLossModel.empty()) {
                wifiChannelintra.AddPropagationLoss(clusterPropagationLossModel);
            }

            // Setup the PHY layer for this cluster with the unique channel
            YansWifiPhyHelper wifiPhyCluster;
            wifiPhyCluster.SetChannel(wifiChannelintra.Create());

            // Configure SSID for the cluster
            std::ostringstream ssidStream;
            ssidStream << "Cluster_" << i;
            std::string ssid = ssidStream.str();

            // Configure the MAC layer for AdHoc (for both GDT and CH)
            WifiMacHelper wifiMacCM;
            wifiMacCM.SetType(clusterMacType, "Ssid", SsidValue(Ssid(ssid)));

            // Install WiFi devices on nodes in the current cluster
            NetDeviceContainer clusterDevices = localWifiIntra.Install(wifiPhyCluster, wifiMacCM, clusters[i]);

            for (uint32_t j = 0; j < clusterDevices.GetN(); j++) {
    
                // Grab the generic device
                Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(clusterDevices.Get(j));
                
                if (wifiDev) {
                    // Cast the generic MAC into our custom TdmaWifiMac
                    Ptr<TdmaWifiMac> tdmaMac = DynamicCast<TdmaWifiMac>(wifiDev->GetMac());
                    
                    if (tdmaMac) {
                        // Inject the JSON profiles directly into the hardware's memory!
                        tdmaMac->SetTrafficProfiles(this->m_deviceTrafficProfiles);
                    }
                }
            }

            this->clustersDevices.push_back(clusterDevices);
        }

        NS_LOG_DEBUG("Devices for intra-cluster communication installed on the nodes");
    }

    // Create the link for nodes to the GDT to prepare for dynamic assignment of CH during the simulation
    // void FANETDeviceHelper::SetUpLinksWifi(Ptr<FANETTopologyHelper> fanet) {
    //     // Ensure the WiFi standard is set
    //     wifi.SetStandard(linkWifiStandard);
    //     //wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("DsssRate11Mbps"), "ControlMode", StringValue("DsssRate11Mbps"));

    //     for (size_t i = 0; i < fanet->clusters.size(); i++){
    //         std::vector<NetDeviceContainer> clusterLinks;
    //         this->clustersLinkDevices.push_back(clusterLinks);
    //     }

    //     // Iterate over each cluster, and create the link between each cluster's node and the GDT
    //     for (size_t i = 0; i < fanet->clusters.size(); i++) {
    //         // Iterate over each node in the cluster
            
    //         for (uint32_t j = 0; j < fanet->clusters[i].GetN(); j++)
    //         {
                
    //             // Create a separate WiFi channel for the link between cluster node and GDT 
    //             YansWifiChannelHelper wifiChannelLink;
    //             if (!linkWifiChannelPropagationDelay.empty()) {
    //                 wifiChannelLink.SetPropagationDelay(clusterWifiChannelPropagationDelay);
    //             }
    //             if (!linkPropagationLossModel.empty()) {
    //                 wifiChannelLink.AddPropagationLoss(clusterPropagationLossModel);
    //             }

    //             // Setup the PHY layer for CH-GDT links
    //             YansWifiPhyHelper wifiPhyLink;
    //             wifiPhyLink.SetChannel(wifiChannelLink.Create());
    //             // Create a unique SSID for each CH-GDT pair
    //             std::ostringstream ssidStream;
    //             ssidStream << "Link_" << i << "_" << j;
    //             std::string ssid = ssidStream.str();

    //             // Configure the MAC layer for AdHoc (for both GDT and CH)
    //             WifiMacHelper wifiMacAdHoc;
    //             wifiMacAdHoc.SetType(linkMacType, "Ssid", SsidValue(Ssid(ssid)));

    //             // Install the WiFi device on the GDT node (AdHoc mode)
    //             NetDeviceContainer adhocDeviceGDT = wifi.Install(wifiPhyLink, wifiMacAdHoc, fanet->GDTNode.Get(0));  

    //             // Install the WiFi device on the CH node (AdHoc mode)
    //             NetDeviceContainer adhocDeviceClusterNode = wifi.Install(wifiPhyLink, wifiMacAdHoc, fanet->clusters[i].Get(j));

    //             // Combine the devices into a single container for this link
    //             NetDeviceContainer linkDevices;
    //             linkDevices.Add(adhocDeviceGDT);
    //             linkDevices.Add(adhocDeviceClusterNode);

    //             this->clustersLinkDevices[i].push_back(linkDevices);
    //         }
    //     }

    //     NS_LOG_DEBUG("Devices for GDT-Cluster communication installed on the nodes");
    // }

    //Creates a channel representing inter-cluster(f_0), attach it to the GDT and all nodes
    void FANETDeviceHelper::SetUpLinksWifi(Ptr<FANETTopologyHelper> fanet) {
        
        WifiHelper localWifiInter;
        localWifiInter.SetStandard(linkWifiStandard);

        YansWifiChannelHelper wifiChannelinter;
    
        if (!linkWifiChannelPropagationDelay.empty()) {
            wifiChannelinter.SetPropagationDelay(linkWifiChannelPropagationDelay);
        }
        if (!linkPropagationLossModel.empty()) {
            wifiChannelinter.AddPropagationLoss(linkPropagationLossModel);
        }

        YansWifiPhyHelper wifiPhyInter;
        wifiPhyInter.SetChannel(wifiChannelinter.Create());

        // Configure 1 common MAC and SSID for the Inter-cluster network
        WifiMacHelper wifiMacInter;
        wifiMacInter.SetType(linkMacType, "Ssid", SsidValue(Ssid("InterCluster_f0")));

        // Install this f0 radio on the GDT
        NetDeviceContainer gdtInterDevice = localWifiInter.Install(wifiPhyInter, wifiMacInter, fanet->GDTNode.Get(0));    
        // If you have a variable to hold the GDT device, save it here
        this->GDTDevice = gdtInterDevice; 

        // Install this SAME f0 radio on ALL cluster nodes.
        // CH assignment is dynamic, every node needs the hardware, 
        // but the TDMA MAC logic will keep it silent unless they are promoted to CH.
        for (size_t i = 0; i < fanet->clusters.size(); i++) {
        
            // Install on the whole cluster at once
            NetDeviceContainer clusterInterDevices = localWifiInter.Install(wifiPhyInter, wifiMacInter, fanet->clusters[i]);
        
            for (uint32_t j = 0; j < clusterInterDevices.GetN(); j++) {
    
                // Grab the generic device
                Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(clusterInterDevices.Get(j));
                
                if (wifiDev) {
                    // Cast the generic MAC into our custom TdmaWifiMac
                    Ptr<TdmaWifiMac> tdmaMac = DynamicCast<TdmaWifiMac>(wifiDev->GetMac());
                    
                    if (tdmaMac) {
                        // Inject the JSON profiles directly into the hardware's memory!
                        tdmaMac->SetTrafficProfiles(this->m_deviceTrafficProfiles);
                    }
                }
            }

            // Create an inner vector to satisfy the 2D requirement of clustersLinkDevices
            std::vector<NetDeviceContainer> innerVector;
            innerVector.push_back(clusterInterDevices);

            // Store the devices. Note: I changed this to push back the whole container
            // rather than doing it node-by-node to match standard ns-3 topology structures.
            this->clustersLinkDevices.push_back(innerVector);
        }

    NS_LOG_DEBUG("Devices for GDT-Cluster communication (f_0) installed");
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

        NS_LOG_DEBUG("Slots allocated to each node for TDMA");
    }

    void FANETDeviceHelper::AssignClusterHeads(Ptr<FANETTopologyHelper> fanet, Ptr<FANETAddressHelper> ipv4, FANETAnimationHelper* anim)
    {
        fanet->CHNodes.clear(); // Clear previous CH assignments before reassigning

        for (size_t i = 0; i < fanet->clusters.size(); i++)
        {
            // Obtain the closest node of the cluster to the GDT 
            Ptr<Node> closestNode = FANETMobilityHelper::GetClosestNode(fanet->GDTNode.Get(0), fanet->clusters[i]);

            if (closestNode==nullptr && fanet->clusters[i].GetN() > 0) {
                closestNode = fanet->clusters[i].Get(0); // Fallback to the first node if GetClosestNode fails
            }
            fanet->CHNodes.push_back(closestNode); // Store the closest node as the CH for this cluster
            
            if (closestNode != nullptr) {
                 // Notify the application layer of the CH assignment
                NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
                        << closestNode->GetId() << " selected as cluster head");
            }
        }
        if (anim != nullptr && !fanet->CHNodes.empty()) {
            anim->AssignCHAnim(fanet->CHNodes);
        }
        
        Simulator::Schedule(Seconds(5.0), &FANETDeviceHelper::ReassignClusterHeads, this, fanet, ipv4, anim);
    }

    void FANETDeviceHelper::ReassignClusterHeads(Ptr<FANETTopologyHelper> fanet, Ptr<FANETAddressHelper> ipv4, FANETAnimationHelper* anim)
    {
        for (size_t i = 0; i < fanet->clusters.size(); i++)
        {
            Ptr<Node> closestNode = FANETMobilityHelper::GetClosestNode(fanet->GDTNode.Get(0), fanet->clusters[i]);
            
            if (closestNode == nullptr && fanet->clusters[i].GetN() > 0) {
                closestNode = fanet->clusters[i].Get(0); // Fallback to the first node if GetClosestNode fails
            }
            
            if (closestNode == nullptr || i > fanet->CHNodes.size() || fanet->CHNodes[i] == nullptr) continue; 
            Ptr<Node> clusterHeadNode = fanet->CHNodes[i];

            if (closestNode->GetId() != clusterHeadNode->GetId()) {

                fanet->CHNodes[i] = closestNode; // Update the CH assignment for this cluster

                 // Notify the application layer of the CH reassignment 
                NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
                            << closestNode->GetId() << " reassigned as cluster head");
            }
        }

        if (anim != nullptr) {
            anim->UpdateCHAnim(fanet->CHNodes);
        }

        Simulator::Schedule(Seconds(5.0), &FANETDeviceHelper::ReassignClusterHeads, this, fanet, ipv4, anim);       
    }

    void FANETDeviceHelper::SetTrafficProfiles(std::vector<TrafficProfile> profiles) {
    this->m_deviceTrafficProfiles = profiles;
}

    void FANETDeviceHelper::NotifyCHStatusChange(Ptr<Node> node, std::string status)
    {
        // Ptr<Socket> socket = Socket::CreateSocket(node, UdpSocketFactory::GetTypeId());
        // InetSocketAddress addr = InetSocketAddress(Ipv4Address("127.0.0.1"), 8080);

        // FANETHeader header;
        // header.SetNodeId(node->GetId());
        // header.SetClusterId(999);
        // header.SetType(HELLO);
        // header.SetService(CH_PROMO);

        // Ptr<Packet> packet = Create<Packet>((uint8_t*) status.c_str(), status.length());
        // packet->AddHeader(header);

        // socket->Connect(addr);
        // socket->Send(packet);
        // socket->Close();

        //NS_LOG_DEBUG("Node " << node->GetId() <<" notifying application of CH status change");
    }
}

