#include "FANETSimulator.h"
#include "ns3/applications-module.h"
#include "ns3/add-client.h"
#include "ns3/add-server.h"
#include "ns3/cluster-node-app.h"
#include "ns3/gdt-app.h"
#include "ns3/FANETAppHelper.h"
#include "ns3/json.hpp"

namespace ns3 
{

    NS_LOG_COMPONENT_DEFINE("FANETSimulator");

    using json = nlohmann::json;

    TypeId FANETSimulator::GetTypeId()
    {
        static TypeId tid = 
            TypeId("ns3::FANETSimulator")
                .SetParent<ns3::Object>()
                .AddConstructor<FANETSimulator>()
                .AddAttribute(  "nClusters",
                                "The number of clusters in the FANET Network",
                                UintegerValue(3),
                                MakeUintegerAccessor(&FANETSimulator::nClusters),
                                MakeUintegerChecker<uint32_t>())
                .AddAttribute(  "nClusterNodes",
                                "Space-separated list of number of nodes in each cluster: 'x1 y1 z1 x2 y2 z2 ...'",
                                StringValue("3 3 3"),
                                MakeStringAccessor(&FANETSimulator::nClusterNodesString),
                                MakeStringChecker())
                .AddAttribute(  "cycleDuration",
                                "Period of each cycle",
                                UintegerValue(200),
                                MakeUintegerAccessor(&FANETSimulator::cycleDuration),
                                MakeUintegerChecker<uint32_t>())
                .AddAttribute(  "filename",
                                "Output .xml file name for NetAnim",
                                StringValue("animation.xml"),
                                MakeStringAccessor(&FANETSimulator::fileName),
                                MakeStringChecker())
                .AddAttribute(  "simulationDuration",
                                "Duration of simulation",
                                DoubleValue(0.0),
                                MakeDoubleAccessor(&FANETSimulator::simDuration),
                                MakeDoubleChecker<double>());

        return tid;
    }

    FANETSimulator::FANETSimulator()
    {

    }

    FANETSimulator::~FANETSimulator()
    {
        delete this->mobility;
    }

    void FANETSimulator::Setup()
    {
        this->fanet = CreateObject<FANETTopologyHelper>();
        if (this->fanet == nullptr) {
        NS_FATAL_ERROR("Failed to create FANETTopologyHelper object!");
        }
        this->fanetDevices = CreateObject<FANETDeviceHelper>();
        this->router = CreateObject<FANETRoutingHelper>();
        this->ipv4 = CreateObject<FANETAddressHelper>();
    }

    void FANETSimulator::GetNClusters()
    {
        std::cout << "Number of clusters to simulate: ";
        std::string input;
        std::getline(std::cin, input);  // Read input as a string

        this->nClusters = std::stoul(input);  // Convert string to uint32_t
    }

    void FANETSimulator::GetNClusterNodes()
    {
        std::string input;
        uint32_t temp;

        for (uint32_t i = 0; i < this->nClusters; i++)
        {
            std::cout << "Number of cluster nodes in Cluster " << i << ": ";
            std::getline(std::cin, input);

            temp = std::stoul(input);
            this->nClusterNodes.push_back(temp);
        }
    }

    void FANETSimulator::SetConstNClusterNodes(uint32_t nClusters, uint32_t nClusterNodes)
    {
        for (uint32_t i = 0; i < nClusters; i++)
            this->nClusterNodes.push_back(nClusterNodes);
    }

    void FANETSimulator::GetCycleDuration()
    {
        std::string input;
        uint32_t temp;

        std::cout << "Duration of each cycle for TDMA (ms) " << ": ";
        std::getline(std::cin, input);

        temp = std::stoul(input);
        this->cycleDuration = temp;
    }

    void FANETSimulator::GetSimulationDuration()
    {
        std::string input;
        double duration;

        std::cout << "Duration of simulation (s) : ";
        std::getline(std::cin, input);

        duration = std::stod(input);
        this->simDuration = duration;
    }

    void FANETSimulator::CreateNetwork()
    {
        
        //this->mobility = new FANETMobilityHelper();
        this->fanet->CreateFANET(this->nClusters, this->nClusterNodes);
    }

    void FANETSimulator::SetMobility()
    {
        this->mobility = new FANETMobilityHelper();
        mobility->ApplyMobilityWireless(this->fanet);
    }

    void FANETSimulator::InstallDevices()
    {
        //this->fanetDevices->SetupGDTWifi(this->fanet->GDTNode);
        this->fanetDevices->SetupClustersWifi(this->fanet->clusters);
        this->fanetDevices->SetUpLinksWifi(this->fanet);
        this->fanetDevices->AssignTdmaSlots(this->fanet, 
                                            MilliSeconds(this->cycleDuration),
                                            this->m_intraClusterConfigs, 
                                            this->m_interClusterConfigs);
        //Print the GDT interfaces
        Ptr<Node> gdt = this->fanet->GDTNode.Get(0);
        std::cout << "GDT (Node " << gdt->GetId() << ") has " << gdt->GetNDevices() << " hardware interfaces." << std::endl;
    }

    void FANETSimulator::SetRoutingProtocol()
    {
        this->router->InstallRoutingProtocol(this->fanet->allNodes);

    }

    void FANETSimulator::AssignAddress()
    {
        this->ipv4->SetBases(this->fanetDevices->GDTDevice,  this->fanetDevices->clustersDevices, this->fanetDevices->clustersLinkDevices);
    }

    void FANETSimulator::SetUpNetAnim()
    {
        this->anim = new FANETAnimationHelper(this->fileName);
        anim->SetMaxPktsPerTraceFile(5000000);
    }

    void FANETSimulator::ParseClusterNodesString()
    {
        std::stringstream ss(nClusterNodesString);
        uint32_t value;
        while (ss >> value) nClusterNodes.push_back(value);

    }

    void FANETSimulator::RunSimulation()
    {
        ns3::PacketMetadata::Enable();

        // See if the Application is successfully pushing data out
        //LogComponentEnable("UdpSocketImpl", LOG_LEVEL_INFO); 
        
        // See if AODV is desperately crying out for a route but failing
        //LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_LOGIC); 
        
        //See if the TDMA MAC layer is properly prioritizing and scheduling packets according to the traffic profiles
        //LogComponentEnable("TdmaWifiMac", LOG_LEVEL_FUNCTION);

        this->CreateNetwork();

        this->SetMobility();

        size_t actualClusters = this->fanet->clusters.size();
    
        // Resize the deques to match the exact number of created clusters
        m_intraClusterConfigs.resize(actualClusters);
        m_interClusterConfigs.resize(actualClusters);
        m_chIntraConfigs.resize(actualClusters);

        //Filter Video out for the CH Local Antenna
        std::vector<TrafficProfile> chBaselineProfiles;
        for (const auto& tp : this->m_trafficProfiles) {
            if (tp.type.find("Video") == std::string::npos) {
                chBaselineProfiles.push_back(tp);
            }
        }

        //Aggregate profiles for the CH Backbone Antenna
        //Assuming 4 nodes per cluster
        std::vector<TrafficProfile> aggregatedBaseline;
        for (const auto& tp : this->m_trafficProfiles) {
            TrafficProfile aggDemand = tp;
            aggDemand.bandwidthKb = (tp.bandwidthKb * 4.0);
            aggregatedBaseline.push_back(aggDemand);
        }

        // Initialize the TDMA MAC configurations for each cluster
        for (size_t i = 0; i < actualClusters; i++) {
            // Intra-cluster settings
            m_intraClusterConfigs[i].totalMiniSlots = 12;
            m_intraClusterConfigs[i].kbPerMiniSlot = 0.1;
            // Assign the baseline traffic profiles so the MAC is active at T=0
            m_intraClusterConfigs[i].trafficProfiles = this->m_trafficProfiles; 

            //Intra-cluster settings for the cluster head
            m_chIntraConfigs[i].totalMiniSlots = 12;
            m_chIntraConfigs[i].kbPerMiniSlot = 0.1;
            m_chIntraConfigs[i].trafficProfiles = chBaselineProfiles;

            // Inter-cluster settings
            m_interClusterConfigs[i].totalMiniSlots = 24;
            m_interClusterConfigs[i].kbPerMiniSlot = 0.1;
            m_interClusterConfigs[i].trafficProfiles = aggregatedBaseline;
        }

        this->InstallDevices();

        this->SetRoutingProtocol();

        this->AssignAddress();
 
        // node = this->fanet->clusters[1].Get(2);
        // Ptr<ClusterNodeApp> app = DynamicCast<ClusterNodeApp>

        LogComponentDisable("ClusterNodeCHPromo", LOG_LEVEL_DEBUG);

        this->SetUpNetAnim();

        //Schedule the cluster head assignment to run shortly after the simulation starts 
        //to ensure all devices are installed and ready, but before the applications start sending data, 
        //so that the cluster heads are properly assigned and can manage the TDMA scheduling from the get-go.
        Simulator::Schedule(Seconds(0.001), &FANETDeviceHelper::AssignClusterHeads, this->fanetDevices, this->fanet, this->ipv4, this->anim);
        //Schedule the periodic topology sync to run every 2ms to print the TDMA grid map of each node and verify that the mini-slot allocations are correct and updating as expected based on the traffic profiles.
        Simulator::Schedule(Seconds(0.002), &FANETSimulator::PeriodicTopologySync, this);

        //Dynamically fetch the GDT's IP address to use as the destination for the applications instead of hardcoding it. 
        //This also serves as a demonstration of how the GDT can be aware of the cluster nodes and their addresses right from the start, 
        //which is crucial for the GDT to perform its management functions effectively.
        Ptr<Node> gcsNode = this->fanet->GDTNode.Get(0);
        Ptr<Ipv4> gdtIpv4 = gcsNode->GetObject<Ipv4>();
        //Interface 1 is the physical f_0 radio linking to the FANET
        Ipv4Address gcsIp = gdtIpv4->GetAddress(1, 0).GetLocal(); 
        
        std::cout << "[APPLICATION] GDT Target IP Address is: " << gcsIp << std::endl;

        //Set up a PacketSink on the GDT to receive the traffic from the cluster nodes and verify that data is being received.
        uint16_t port = 9999;
        PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApp = sinkHelper.Install(gcsNode);
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(this->simDuration));

        //Configure the traffic profiles for the applications based on the JSON configuration. 
        //For simplicity, just set up three types of traffic: Video, Status, and Command, each with different bandwidth requirements and priorities. 
        //The TDMA MAC layer will use the priorities to allocate mini-slots accordingly.
        //VIDEO: 1.2K High Res -> 1200 bytes, 9.6Kbps | TID 5 (DSCP 0xA0)
        OnOffHelper videoApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        videoApp.SetConstantRate(DataRate("500Kbps"), 1200); 
        videoApp.SetAttribute("Tos", UintegerValue(0xA0)); 

        //STATUS 1: 0.1K -> 100 bytes, 0.8Kbps | TID 4 (DSCP 0x80)
        OnOffHelper statusApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        statusApp.SetConstantRate(DataRate("0.8Kbps"), 100); 
        statusApp.SetAttribute("Tos", UintegerValue(0x80)); 

        //CMD 1: 0.1K -> 100 bytes, 0.8Kbps | TID 6 (DSCP 0xC0)
        OnOffHelper cmdApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        cmdApp.SetConstantRate(DataRate("0.8Kbps"), 100); 
        cmdApp.SetAttribute("Tos", UintegerValue(0xC0));

        //Isntall applications on all the cluster nodes with staggered start times to prevent collisions and ensure the GDT is ready to receive when the apps start sending
        for (size_t i = 0; i < this->fanet->clusters.size(); i++) {
            for (uint32_t j = 0; j < this->fanet->clusters[i].GetN(); j++) {
                
                Ptr<Node> currentNode = this->fanet->clusters[i].Get(j);
                
                //Stagger the start times to prevent catastrophic AODV route-request collisions
                double staggerOffset = (i * 0.1) + (j * 0.05); 

                //Install Video
                ApplicationContainer vApp = videoApp.Install(currentNode);
                vApp.Start(Seconds(1.0 + staggerOffset)); 
                vApp.Stop(Seconds(this->simDuration));

                //Install Status
                ApplicationContainer sApp = statusApp.Install(currentNode);
                sApp.Start(Seconds(1.1 + staggerOffset));
                sApp.Stop(Seconds(this->simDuration));

                //Install Cmd
                ApplicationContainer cApp = cmdApp.Install(currentNode);
                cApp.Start(Seconds(1.2 + staggerOffset));
                cApp.Stop(Seconds(this->simDuration));
            }
        }

        //Schedule the dynamic update of traffic profiles based on the JSON configuration.
        for (const auto& block : this->m_scheduledtrafficWindows) 
        {
            // Schedule the activation of this specific profile block
            Simulator::Schedule(Seconds(block.startTime), 
                                &FANETSimulator::ExecuteProfileSwap, this, 
                                block.profiles, "ACTIVATING UPDATE PROFILE WINDOW");

            // Schedule the automatic tear-down to revert back to baseline trafficProfiles
            Simulator::Schedule(Seconds(block.endTime), 
                                &FANETSimulator::ExecuteProfileSwap, this, 
                                this->m_trafficProfiles, "WINDOW ENDED - REVERTING TO BASELINE");

        }
    
        this->anim->AnimateFANET(this->fanet);

        // Give the GDT an active schedule so it can transmit Commands and ARP replies
        Ptr<Node> activeGcsNode = this->fanet->GDTNode.Get(0);
        for (uint32_t d = 0; d < activeGcsNode->GetNDevices(); d++) {
            Ptr<WifiNetDevice> gdtWifi = DynamicCast<WifiNetDevice>(activeGcsNode->GetDevice(d));
            if (gdtWifi) {
                Ptr<TdmaWifiMac> gdtMac = DynamicCast<TdmaWifiMac>(gdtWifi->GetMac());
                if (gdtMac) {
                    gdtMac->SetClusterConfig(&m_interClusterConfigs[0]);
                }
            }
        }

        //Set up Node listener dynamically to listen for incoming commands from the GCS 
        Ptr<Socket> cmdSocket = Socket::CreateSocket(targetNode, UdpSocketFactory::GetTypeId());
        cmdSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), this->m_targetPort));
        cmdSocket->SetRecvCallback(MakeCallback(&FANETSimulator::CommandCallBack, this));

        //Get the IP address of that target node so the GCS knows where to aim
        Ptr<Ipv4> ipv4 = targetNode ->GetObject<Ipv4>();
        Ipv4Address targetIp = ipv4 ->GetAddress(2, 0).GetLocal();

        //Create the GDTapp 
        Ptr<GDTApp> gdtApp = CreateObject<GDTApp>();
        gcsNode->AddApplication(gdtApp);
        gdtApp->SetStartTime(Seconds(0.0));
        gdtApp->SetStopTime(Seconds(this->simDuration));

        Ptr<NetDevice> gdtInterClusterRadio;
        for (uint32_t d = 0; d < gcsNode->GetNDevices(); d++) {
            Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(gcsNode->GetDevice(d));
            if (wifiDev) {
                std::string ssid = wifiDev->GetMac()->GetSsid().PeekString();
                if (ssid.find("InterCluster") != std::string::npos) {
                    gdtInterClusterRadio = wifiDev;
                    break;
                }
            }
        }
        //Schedule the GDT App to dispatch the command through the Inter-Cluster radio
        Simulator::Schedule(Seconds(this->m_updateTime), 
                            &GDTApp::SendCommand, 
                            gdtApp, targetIp, this->m_targetPort, this->m_commandString, gdtInterClusterRadio);

        Simulator::Stop(Seconds(simDuration));
        Simulator::Run();

        //The verification of GDT receiving data can be done by checking the total bytes received in the sink application on the GDT node. 
        //If it's greater than 0, it means the GDT successfully received some data from the Nodes.
        Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
        std::cout << "\n" << std::endl;
        std::cout << "[VERIFICATION] GDT successfully received: " << sink->GetTotalRx() << " bytes." << std::endl;
        std::cout << "\n" << std::endl;

        Simulator::Destroy();
    }

    void FANETSimulator::SetupSimulation(std::string jsonFilePath)
    {
        std::ifstream file(jsonFilePath);
        json config;
        file >> config;

        //Read the Bandwidth Table from JSON
        if (config.contains("trafficProfiles")) {
            for (const auto& item : config["trafficProfiles"]) {
                TrafficProfile tp;
                tp.type = item["type"].get<std::string>();
                tp.priority = item["priority"].get<uint32_t>();
                tp.bandwidthKb = item["bandwidthKb"].get<double>();
            
                this->m_trafficProfiles.push_back(tp);
            }
        }
    
        //Sort the profiles by priority (1 comes first) so the MAC layer handles them in order
        std::sort(this->m_trafficProfiles.begin(), this->m_trafficProfiles.end(),
                [](const TrafficProfile& a, const TrafficProfile& b) {
                    return a.priority < b.priority;
                });
        
        this->m_scheduledtrafficWindows.clear(); //clear existing blocks

        //Read update profile from json
        if (config.contains("updateProfiles")) {
            for (const auto& block : config["updateProfiles"]) {
                TrafficWindow tw;
                tw.startTime = block["startTime"].get<double>();
                tw.endTime = block["endTime"].get<double>();
                
                //Parse the traffic profiles for this update block
                for (const auto& item : block["profiles"]) {
                    TrafficProfile tp;
                    tp.type = item["type"].get<std::string>();
                    tp.priority = item["priority"].get<uint32_t>();
                    tp.bandwidthKb = item["bandwidthKb"].get<double>();
                    tw.profiles.push_back(tp);
                    this->m_updateProfiles.push_back(tp);
                }

                //Sort the profiles within this block by priority
                std::sort(tw.profiles.begin(), tw.profiles.end(),
                    [](const TrafficProfile& a, const TrafficProfile& b) {
                        return a.priority < b.priority;
                    });

                //Add this block to the list of update profiles
                this->m_scheduledtrafficWindows.push_back(tw);
            }
        }
    
        //Sort the update profiles by priority
        std::sort(this->m_updateProfiles.begin(), this->m_updateProfiles.end(),
                [](const TrafficProfile& a, const TrafficProfile& b) {
                    return a.priority < b.priority;
                });

        // Read dynamic injection time from json (default set to 15s if not provided)
        if (config.contains("updateTime")) {
            this->m_updateTime = config["updateTime"].get<double>();
        } else {
            this->m_updateTime = 15.0; 
        }

        //Parse GDT command configuration from JSON. 
        //This includes which node should be the target of the command, what port to listen on, and what command string to send.
        if (config.contains("gdtCommandConfig")) {
            this->m_targetClusterIndex = config["gdtCommandConfig"]["targetClusterIndex"].get<uint32_t>();
            this->m_targetNodeIndex = config["gdtCommandConfig"]["targetNodeIndex"].get<uint32_t>();
            this->m_targetPort = config["gdtCommandConfig"]["targetPort"].get<uint16_t>();
            this->m_commandString = config["gdtCommandConfig"]["commandString"].get<std::string>();
        } else {
            //Safe defaults if the block is missing from the JSON
            this->m_targetClusterIndex = 0;
            this->m_targetNodeIndex = 0;
            this->m_targetPort = 9999;
            this->m_commandString = "HIGH_RES";
        }

        Setup();
        SetAttribute("nClusters", UintegerValue(config["nClusters"]));
        SetAttribute("nClusterNodes", StringValue(config["nClusterNodes"].get<std::string>()));
        ParseClusterNodesString();
        SetAttribute("cycleDuration", UintegerValue(config["cycleDuration"]));
        SetAttribute("filename", StringValue(config["filename"].get<std::string>()));
        SetAttribute("simulationDuration", DoubleValue(config["simulationDuration"]));; 
 
        fanetDevices->SetAttribute("clusterWifiStandard", EnumValue(wifiStandardMap[config["fanetDevices"]["clusterWifiStandard"].get<std::string>()]));
        fanetDevices->SetAttribute("clusterWifiChannelPropagationDelay", StringValue(config["fanetDevices"]["clusterWifiChannelPropagationDelay"].get<std::string>()));
        fanetDevices->SetAttribute("clusterPropagationLossModel", StringValue(config["fanetDevices"]["clusterPropagationLossModel"].get<std::string>()));
        fanetDevices->SetAttribute("clusterMacType", StringValue(config["fanetDevices"]["clusterMacType"].get<std::string>()));
        fanetDevices->SetAttribute("linkWifiStandard", EnumValue(wifiStandardMap[config["fanetDevices"]["linkWifiStandard"].get<std::string>()]));
        fanetDevices->SetAttribute("linkWifiChannelPropagationDelay", StringValue(config["fanetDevices"]["linkWifiChannelPropagationDelay"].get<std::string>()));
        fanetDevices->SetAttribute("linkPropagationLossModel", StringValue(config["fanetDevices"]["linkPropagationLossModel"].get<std::string>()));
        fanetDevices->SetAttribute("linkMacType", StringValue(config["fanetDevices"]["linkMacType"].get<std::string>()));

        router->SetAttribute("routingProtocol", EnumValue(routingProtocolMap[config["router"]["routingProtocol"].get<std::string>()]));
        
        ipv4->SetAttribute("baseNetworkAddress", Ipv4AddressValue(Ipv4Address(config["ipv4"]["baseNetworkAddress"].get<std::string>().c_str())));
        ipv4->SetAttribute("baseSubnetMask", Ipv4MaskValue(Ipv4Mask(config["ipv4"]["baseSubnetMask"].get<std::string>().c_str())));

        this->m_currentActiveProfiles = this->m_trafficProfiles; //Initialize the current active profiles to the baseline profiles at the start of the simulation
    }

    //This callback is triggered when the target node receives the command from the GCS. 
    //It identifies which cluster the node belongs to, updates the central configuration objects 
    //with the new traffic profiles, and signals all nodes in that cluster to refresh their TDMA slot allocations based on the new profiles.
    void FANETSimulator::CommandCallBack(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        Address from;
        while ((packet = socket->RecvFrom(from)))
        {
            Ptr<Node> rxNode = socket->GetNode();
            uint32_t rxNodeId = rxNode->GetId();

            //Identify which cluster this node belongs to by searching through the clusters and their member nodes to find a match for the receiving node's ID. This is necessary to know which cluster's configuration
            int targetClusterId = -1;
            uint32_t clusterSize = 0;
            for (size_t i = 0; i < this->fanet->clusters.size(); i++) {
                for (uint32_t j = 0; j < this->fanet->clusters[i].GetN(); j++) {
                    if (this->fanet->clusters[i].Get(j)->GetId() == rxNodeId) {
                        targetClusterId = i;
                        clusterSize = this->fanet->clusters[i].GetN();
                        break;
                    }
                }
                if (targetClusterId != -1) break;
            }

            if (targetClusterId < 0 || (size_t)targetClusterId >= m_intraClusterConfigs.size()) {
                NS_LOG_ERROR("FATAL: targetClusterId " << targetClusterId << " is out of bounds!");
                continue; // Skip this packet if the cluster ID is invalid
            }

            std::cout << "\n[NODE LISTENER] Time: " << Simulator::Now().As(Time::S) << std::endl;
            std::cout << "[NODE LISTENER] Node " << rxNodeId << " received command. Updating Central Cluster Configuration Object..." << std::endl;

            //Update the central cluster configuration objects with the new traffic profiles based on the received command. 
            this->m_intraClusterConfigs[targetClusterId].trafficProfiles = this->m_updateProfiles;

            //Filter Video for CH
            std::vector<TrafficProfile> chIntraProfiles;
            for (const auto& p : this->m_updateProfiles) {
                if (p.type.find("Video") == std::string::npos) {
                    chIntraProfiles.push_back(p);
                }
            }
            this->m_chIntraConfigs[targetClusterId].trafficProfiles = chIntraProfiles;

            //Generate the aggregated traffic profiles for the inter-cluster communication by multiplying the bandwidth requirements 
            //of each profile by the number of nodes in the cluster, since the cluster head will be handling the traffic for 
            //all its members when communicating with other clusters.
            std::vector<TrafficProfile> aggregatedProfiles;
            for (const auto& baseProfile : this->m_updateProfiles) {
                TrafficProfile clusterDemand = baseProfile;
                clusterDemand.bandwidthKb = (baseProfile.bandwidthKb * clusterSize)-0.001; 
                aggregatedProfiles.push_back(clusterDemand);
            }
            this->m_interClusterConfigs[targetClusterId].trafficProfiles = aggregatedProfiles;

            //Signal all nodes in this cluster to refresh their TDMA mini-slot allocations based on the updated traffic profiles.
            for (uint32_t j = 0; j < this->fanet->clusters[targetClusterId].GetN(); j++) {
                Ptr<Node> clusterNode = this->fanet->clusters[targetClusterId].Get(j);
                uint32_t nodeId = clusterNode->GetId();

                //Determine if this node is the cluster head, 
                //since the cluster head will have a different configuration for its inter-cluster interface compared to the regular member nodes.
                bool isClusterHead = false;
                for (size_t c = 0; c < this->fanet->CHNodes.size(); c++) {
                    if (this->fanet->CHNodes[c] != nullptr && this->fanet->CHNodes[c]->GetId() == nodeId) {
                        isClusterHead = true;
                        break;
                    }
                }

                //Iterate through the devices of this node to find the WiFi interfaces and update their TDMA MAC configurations 
                //based on whether they are inter-cluster or intra-cluster interfaces, and whether this node is a cluster head or not.
                for (uint32_t d = 0; d < clusterNode->GetNDevices(); d++) {
                    Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(clusterNode->GetDevice(d));
                    if (wifiDev) {
                        Ptr<TdmaWifiMac> tdmaMac = DynamicCast<TdmaWifiMac>(wifiDev->GetMac());
                        if (tdmaMac) {
                            std::string ssid = wifiDev->GetMac()->GetSsid().PeekString();
                            bool isInterCluster = (ssid.find("InterCluster") != std::string::npos);

                            if (isInterCluster) {
                                if (isClusterHead) {
                                    //Get aggregated video profile
                                    tdmaMac->SetClusterConfig(&m_interClusterConfigs[targetClusterId]);
                                } else {
                                    tdmaMac->SetClusterConfig(nullptr);
                                }
                            } else {
                                if (isClusterHead)
                                {
                                    //Get No Video Profile
                                    tdmaMac->SetClusterConfig(&m_chIntraConfigs[targetClusterId]);
                                }else{                                   
                                    //Get standard video profile
                                    tdmaMac->SetClusterConfig(&m_intraClusterConfigs[targetClusterId]);
                                }
                            }
                            tdmaMac->AllocateMiniSlots();//Recalculate the mini-slot allocations based on the new profiles
                            PrintTdmaGridMap(clusterNode, wifiDev, tdmaMac);
                        }
                    }
                }
            }
            //Send a reply back to the GDT to confirm that the command was received and processed successfully.
            InetSocketAddress senderInetAddr = InetSocketAddress::ConvertFrom(from);
            Ipv4Address gdtIp = senderInetAddr.GetIpv4();
            
            std::cout << "[NODE LISTENER] Sending requested data back to GDT IP: " << gdtIp << std::endl;
            
            Ptr<Socket> replySocket = Socket::CreateSocket(rxNode, UdpSocketFactory::GetTypeId());
            replySocket->Connect(InetSocketAddress(gdtIp, 9999)); 
            
            Ptr<Packet> replyData = Create<Packet>((uint8_t*)"VIDEO_DATA_REPLY", 16);
            replySocket->Send(replyData);
        }
    }

    void FANETSimulator::ExecuteProfileSwap(std::vector<TrafficProfile> profilesToApply, std::string stageName)
    {
        if (!this->fanet || m_intraClusterConfigs.size() != this->fanet->clusters.size()) {
        NS_LOG_WARN("Skipping ProfileSwap: Topology or Config vectors not synchronized. " 
                    << "Topology Clusters: " << (this->fanet ? std::to_string(this->fanet->clusters.size()) : "NULL")
                    << " Config Size: " << m_intraClusterConfigs.size());
        return;
        }
        NS_LOG_UNCOND("[DEBUG] ExecuteProfileSwap called. Cluster count: " << (this->fanet ? this->fanet->clusters.size() : 0));

        for (size_t i = 0; i < this->fanet->clusters.size(); i++) {
            //Dynamically get the number of nodes in this specific cluster
            uint32_t clusterSize = this->fanet->clusters[i].GetN();

            m_intraClusterConfigs[i].trafficProfiles = profilesToApply;

            //Filter Video for CH
            std::vector<TrafficProfile> chIntraProfiles;
            for (const auto& p : profilesToApply) {
                if (p.type.find("Video") == std::string::npos) {
                    chIntraProfiles.push_back(p);
                }
            }
            m_chIntraConfigs[i].trafficProfiles = chIntraProfiles;

            std::vector<TrafficProfile> aggregatedProfiles;
            for (const auto& baseProfile : profilesToApply) {
                TrafficProfile clusterDemand = baseProfile;
                clusterDemand.bandwidthKb = (baseProfile.bandwidthKb * clusterSize) - 0.001;
                aggregatedProfiles.push_back(clusterDemand);
            }
            m_interClusterConfigs[i].trafficProfiles = aggregatedProfiles;
        
            for (uint32_t j = 0; j < clusterSize; j++) {
                Ptr<Node> node = this->fanet->clusters[i].Get(j);
                uint32_t nodeId = node->GetId();
                //Check if this node is currently elected as a Cluster Head
                bool isClusterHead = false;
                for (size_t c = 0; c < this->fanet->CHNodes.size(); c++) {
                    if (this->fanet->CHNodes[c] != nullptr && this->fanet->CHNodes[c]->GetId() == nodeId) {
                        isClusterHead = true;
                        break;
                    }
                }

                //Ensure the node actually has applications installed to prevent crashes
                if (node->GetNApplications() > 0) {
                    //Grab Application 0 (Video App)
                    Ptr<Application> myVideoApp = node->GetApplication(0);
                    
                    if (isClusterHead) {
                        //Mute the video feed, it is a CH and must save bandwidth for routing.
                        myVideoApp->SetAttribute("DataRate", StringValue("1bps"));
                    } else {
                        //Turn the camera on, it is a standard member node.
                        myVideoApp->SetAttribute("DataRate", StringValue("500Kbps"));
                    }
                }

                //Iterate through the devices of this node to find the WiFi interfaces and update their TDMA MAC configurations 
                //based on whether they are inter-cluster or intra-cluster interfaces, and whether this node is a cluster head or not.
                for (uint32_t d = 0; d < node->GetNDevices(); d++) {
                    Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(node->GetDevice(d));
                    if (wifiDev) {
                        Ptr<TdmaWifiMac> tdmaMac = DynamicCast<TdmaWifiMac>(wifiDev->GetMac());
                        if (!tdmaMac) {
                            std::cout << "[DEBUG] Node " << node->GetId() << " Device " << d 
                                    << " is NOT a TdmaWifiMac. Type is: " 
                                    << wifiDev->GetMac()->GetTypeId().GetName() << std::endl;
                            continue;
                        }
                        if (tdmaMac) {
                            std::string ssid = wifiDev->GetMac()->GetSsid().PeekString();
                            bool isInterCluster = (ssid.find("InterCluster") != std::string::npos);

                            //Based on whether this is the inter-cluster or intra-cluster interface, 
                            //and whether this node is a cluster head or not, we apply the appropriate traffic profiles 
                            //and re-allocate the TDMA mini-slots accordingly. Cluster heads get the aggregated profiles on their inter-cluster interfaces, 
                            //plain members get an empty profile to maintain their dormant state on the inter-cluster link, 
                            //and all nodes get the regular profiles on their intra-cluster interfaces.
                            if (isInterCluster) {
                                if (isClusterHead) {
                                    //Get the aggregated profiles for the inter-cluster communication
                                    tdmaMac->SetClusterConfig(&m_interClusterConfigs.at(i));
                                } else {
                                    tdmaMac->SetClusterConfig(nullptr);
                                }
                            } else {
                                if (isClusterHead) {
                                    //Get No Video Profiles for the intra-cluster communication on the cluster head's local antenna, since the CH should not be sending video to its members.
                                    tdmaMac->SetClusterConfig(&m_chIntraConfigs.at(i));
                                } else
                                {
                                    //Get standard video profile
                                    tdmaMac->SetClusterConfig(&m_intraClusterConfigs.at(i));
                                }
                            }
                            tdmaMac->AllocateMiniSlots();
                            PrintTdmaGridMap(node, wifiDev, tdmaMac);
                        }
                    }
                }
            }
            Ptr<Node> syncGcsNode = this->fanet->GDTNode.Get(0);
            for (uint32_t d = 0; d < syncGcsNode->GetNDevices(); d++) {
                Ptr<WifiNetDevice> gdtWifi = DynamicCast<WifiNetDevice>(syncGcsNode->GetDevice(d));
                if (gdtWifi) {
                    Ptr<TdmaWifiMac> gdtMac = DynamicCast<TdmaWifiMac>(gdtWifi->GetMac());
                    if (gdtMac) {
                        gdtMac->SetClusterConfig(&m_interClusterConfigs[0]);
                        gdtMac->AllocateMiniSlots();
                    }
                }
            }
        }
    }


    //Method to periodically synchronize the topology and print the TDMA grid map for each node, demonstrating how the MAC layer adapts to any changes in the network over time (e.g., nodes moving, cluster head changes, etc.)
    void FANETSimulator::PeriodicTopologySync()
    {
        // Re-trigger the profile swap loop to apply rules to any newly promoted/demoted nodes
        ExecuteProfileSwap(this->m_currentActiveProfiles, "PERIODIC TOPOLOGY HARDWARE SYNC");

        // Reschedule the synchronization to execute 5 seconds from now
        Simulator::Schedule(Seconds(5.0), &FANETSimulator::PeriodicTopologySync, this);
    }

    //Enhanced logging function to visualize the TDMA slot allocation in a grid format, along with the node's role and current traffic profile.
    void FANETSimulator::PrintTdmaGridMap(Ptr<Node> node, Ptr<WifiNetDevice> wifiDev, Ptr<TdmaWifiMac> tdmaMac)
    {
        uint32_t nodeId = node->GetId();
        std::string ssid = wifiDev->GetMac()->GetSsid().PeekString();
        std::string simTime = std::to_string((uint32_t)Simulator::Now().GetSeconds()) + "s";
        
        std::stringstream macSs;
        macSs << wifiDev->GetMac()->GetAddress();
        std::string macAddr = macSs.str();

        std::string ipAddr = "Unassigned";
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        
        //
        if (ipv4) {
            //
            int32_t ifIndex = ipv4->GetInterfaceForDevice(wifiDev);
            
            //
            if (ifIndex >= 0 && ipv4->GetNAddresses(ifIndex) > 0) {
                //
                Ipv4Address localIp = ipv4->GetAddress(ifIndex, 0).GetLocal();
                
                //
                std::stringstream ipSs;
                ipSs << localIp;
                ipAddr = ipSs.str();
            }
        }

        //Check if this specific node is an active Cluster Head
        bool isClusterHead = false;
        for (uint32_t i = 0; i < this->fanet->CHNodes.size(); i++) {
            if (this->fanet->CHNodes[i] != nullptr && this->fanet->CHNodes[i]->GetId() == nodeId) {
                isClusterHead = true;
                break;
            }
        }

        bool isInterCluster = (ssid.find("InterCluster") != std::string::npos);
        bool isDormant = isInterCluster && !isClusterHead; 

        //Build Header String
        std::stringstream headerSs;
        if (isDormant) {
            headerSs << "[TDMA HARDWARE STATE CHANGE]  Node: " << nodeId 
                    << "  |  IP: " << ipAddr
                    << "  |  MAC: " << macAddr 
                    << "  |  SSID: " << ssid << " | Sim Time: " << simTime;
        } else {
            headerSs << "[TDMA HARDWARE STATE CHANGE]  Node: " << nodeId 
                    << "  |  IP: " << ipAddr
                    << "  |  MAC: " << macAddr
                    << "  |  SSID: " << ssid << "  | Sim Time: " << simTime;
        }
        std::string headerContent = headerSs.str();

        //Build Grid Map Row
        std::stringstream gridSs;
        gridSs << "GRID MAP | ";

        if (isDormant) {
            gridSs << "[ --- DORMANT INTERFACE (WAITING FOR PROMOTION) --- ]";
        } else {
            //Check if we should print 24 slots or 12 slots
            uint32_t slotsToPrint = isInterCluster ? 24 : 12;
            for (uint32_t i=0; i < slotsToPrint; i++) {
                std::string slotName = tdmaMac->GetSlotTrafficType(i); 
                
                if (slotName == "Status1")              gridSs << "[STA1] ";
                else if (slotName == "Status2")         gridSs << "[STA2] ";
                else if (slotName == "Cmd1")            gridSs << "[CMD1] ";
                else if (slotName == "Cmd2")            gridSs << "[CMD2] ";
                else if (slotName == "Cmd3")            gridSs << "[CMD3] ";
                else if (slotName == "Video_LOW_RES")   gridSs << "[V-LOW] ";
                else if (slotName == "Video_HIGH_RES")  gridSs << "[V-HI] ";
                else                                    gridSs << "[IDLE] ";
            }
        }
        std::string gridContent = gridSs.str();

        size_t internalWidth = std::max(headerContent.length(), gridContent.length()) + 2;

        //Render box
        std::cout << "+" << std::string(internalWidth, '-') << "+" << std::endl;
        std::cout << "| " << headerContent << std::string(internalWidth - headerContent.length() - 1, ' ') << "|" << std::endl;
        std::cout << "+" << std::string(internalWidth, '-') << "+" << std::endl;
        std::cout << "| " << gridContent << std::string(internalWidth - gridContent.length() - 1, ' ') << "|" << std::endl;
        std::cout << "+" << std::string(internalWidth, '-') << "+" << std::endl;
        std::cout << std::endl;
    }  
}

     