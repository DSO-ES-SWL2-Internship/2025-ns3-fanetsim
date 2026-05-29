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
        this->fanet = CreateObject<FANETTopologyHelper>(nClusters, nClusterNodes);
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
        this->fanetDevices->AssignTdmaSlots(this->fanet->allNodes, MilliSeconds(this->cycleDuration));
        //Print the GDT interfaces
        Ptr<Node> gdt = this->fanet->GDTNode.Get(0);
        std::cout << "GDT (Node " << gdt->GetId() << ") has " << gdt->GetNDevices() << " hardware interfaces." << std::endl;
        // Loop to find cluster nodes and print their interfaces and SSIDs
        // for (size_t i = 0; i < this->fanet->clusters.size(); i++) {
        //     for (uint32_t j = 0; j < this->fanet->clusters[i].GetN(); j++) 
        //     {
        //         Ptr<Node> clusterNode = this->fanet->clusters[i].Get(j);
        //         std::cout << "Node " << clusterNode->GetId() << " has " << clusterNode->GetNDevices() << " interfaces. ";
        
        //         for (uint32_t d = 0; d < clusterNode->GetNDevices(); d++) {
        //             Ptr<NetDevice> genericDevice = clusterNode->GetDevice(d);
        //             Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(genericDevice);
    
        //             if (wifiDev) {
        //                 std::cout << "  -> Device " << d << " is Wi-Fi. SSID: " << wifiDev->GetMac()->GetSsid() << std::endl;
        //             } else {
        //                 std::cout << "  -> Device " << d << " is NOT Wi-Fi (Likely Loopback)." << std::endl;
        //             }
        // }
        // std::cout << std::endl;
        //     }
        // }
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

        //this->GetNClusters();

        //this->GetNClusterNodes();

        //this->GetCycleDuration();

        //this->GetSimulationDuration();

        this->CreateNetwork();

        this->SetMobility();

        this->InstallDevices();

        this->SetRoutingProtocol();

        this->AssignAddress();

        // //Setup application on all the nodes
        // InstallApplication<GDTApp>(
        //     this->fanet->GDTNode.Get(0), 0.0, simDuration,
        //     [this](Ptr<GDTApp> app) {
        //         app->SetPort(8080);
        //         app->EnableInfoLog();
        //         app->m_plrManager->Setup(this->fanet->allNodes.GetN());        
        //     }          
        // );
        
        // for (size_t i = 0; i < this->fanet->clusters.size(); i++)
        // {
        //     for (uint32_t j = 0; j < this->fanet->clusters[i].GetN(); j++)
        //     {
        //         InstallApplication<ClusterNodeApp>(
        //             this->fanet->clusters[i].Get(j), 0.0, simDuration,
        //             [this, i](Ptr<ClusterNodeApp> app) {
        //                 app->SetUp(this->ipv4->GDTInterface.GetAddress(0), 8080, i, this->ipv4->GetClusterBaseIP(i));
        //                 //app->SetupPLR(this->fanet->allNodes.GetN());
        //                 app->m_plrManager->Setup(this->fanet->allNodes.GetN());   
        //                 app->EnableInfoLog();
        //             }
        //         ); 
        //     }
        // }

        // // Starting PLR test on some nodes. Note: as of now if there is no mechanism to check what if the response packet from the query was not received.
        // Ptr<Node> node = this->fanet->GDTNode.Get(0);
        // Ptr<FANETApplication> gdtapp = DynamicCast<FANETApplication>(node->GetApplication(0));
        // if (gdtapp) gdtapp->m_plrManager->StartTest(gdtapp, 3, 0, NETWORK_BROADCAST, 20, 0.5);

        // auto& selNode = *this->fanet->clusters[0].Get(0);
        // Ptr<FANETApplication> app = DynamicCast<FANETApplication>(node->GetApplication(0));
        // if (app) app->m_plrManager->StartTest(app, 10, selNode.GetId(), NETWORK_BROADCAST, 20, 0.5);
 
        // node = this->fanet->clusters[1].Get(2);
        // Ptr<ClusterNodeApp> app = DynamicCast<ClusterNodeApp>

        LogComponentDisable("ClusterNodeCHPromo", LOG_LEVEL_DEBUG);

        this->SetUpNetAnim();

        //this->fanetDevices->AssignClusterHeads(this->fanet, this->ipv4, this->anim);
        //try to schedule the cluster head assignment a little later to ensure all the routing tables are populated and the GDT is fully aware of the cluster nodes before it tries to assign them as cluster heads.
        Simulator::Schedule(Seconds(0.001), &FANETDeviceHelper::AssignClusterHeads, this->fanetDevices, this->fanet, this->ipv4, this->anim);

        //DYNAMICALLY FETCH THE GDT'S IP ADDRESS 
        Ptr<Node> gcsNode = this->fanet->GDTNode.Get(0);
        Ptr<Ipv4> gdtIpv4 = gcsNode->GetObject<Ipv4>();
        // Interface 1 is the physical f_0 radio linking to the FANET
        Ipv4Address gcsIp = gdtIpv4->GetAddress(1, 0).GetLocal(); 
        
        std::cout << "[APPLICATION] GDT Target IP Address is: " << gcsIp << std::endl;

        //2. SET UP THE RECEIVER ON THE GDT (Listening on port 9999) 
        uint16_t port = 9999;
        PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApp = sinkHelper.Install(gcsNode);
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(this->simDuration));

        //CONFIGURE THE TRAFFIC GENERATORS (Table 1 Specs) ---
        // VIDEO: 1.2K High Res -> 1200 bytes, 9.6Kbps | Priority 3 (DSCP 0x60)
        OnOffHelper videoApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        videoApp.SetConstantRate(DataRate("9.6Kbps"), 1200); 
        videoApp.SetAttribute("Tos", UintegerValue(0x60)); 

        // STATUS 1: 0.1K -> 100 bytes, 0.8Kbps | Priority 2 (DSCP 0x80)
        OnOffHelper statusApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        statusApp.SetConstantRate(DataRate("0.8Kbps"), 100); 
        statusApp.SetAttribute("Tos", UintegerValue(0x80)); 

        // CMD 1: 0.1K -> 100 bytes, 0.8Kbps | Priority 3 (DSCP 0x60)
        OnOffHelper cmdApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        cmdApp.SetConstantRate(DataRate("0.8Kbps"), 100); 
        cmdApp.SetAttribute("Tos", UintegerValue(0x60)); 

        //INSTALL APPS ON EVERY DRONE IN EVERY CLUSTER ---
        for (size_t i = 0; i < this->fanet->clusters.size(); i++) {
            for (uint32_t j = 0; j < this->fanet->clusters[i].GetN(); j++) {
                
                Ptr<Node> currentDrone = this->fanet->clusters[i].Get(j);
                
                // Stagger the start times to prevent catastrophic AODV route-request collisions
                double staggerOffset = (i * 0.1) + (j * 0.05); 

                // Install Video
                ApplicationContainer vApp = videoApp.Install(currentDrone);
                vApp.Start(Seconds(1.0 + staggerOffset)); 
                vApp.Stop(Seconds(this->simDuration));

                // Install Status
                ApplicationContainer sApp = statusApp.Install(currentDrone);
                sApp.Start(Seconds(1.1 + staggerOffset));
                sApp.Stop(Seconds(this->simDuration));

                // Install Cmd
                ApplicationContainer cApp = cmdApp.Install(currentDrone);
                cApp.Start(Seconds(1.2 + staggerOffset));
                cApp.Stop(Seconds(this->simDuration));
            }
        }

        this->anim->AnimateFANET(this->fanet);

        Simulator::Stop(Seconds(simDuration));
        Simulator::Run();

        // --- THE VERIFICATION PRINT STATEMENT: CHECKING IF THE GDT RECEIVED ANYTHING AT ALL FROM THE CLUSTER NODES ---
        Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
        std::cout << "\n===============================================" << std::endl;
        std::cout << "[VERIFICATION] GDT successfully received: " << sink->GetTotalRx() << " bytes." << std::endl;
        std::cout << "===============================================\n" << std::endl;
 
        // 1. See if the Application is successfully pushing data out
        LogComponentEnable("UdpSocketImpl", LOG_LEVEL_INFO); 
        
        // 2. See if AODV is desperately crying out for a route but failing
        LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_LOGIC);     

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
        
        Setup();
        SetAttribute("nClusters", UintegerValue(config["nClusters"]));
        SetAttribute("nClusterNodes", StringValue(config["nClusterNodes"].get<std::string>()));
        ParseClusterNodesString();
        SetAttribute("cycleDuration", UintegerValue(config["cycleDuration"]));
        SetAttribute("filename", StringValue(config["filename"].get<std::string>()));
        SetAttribute("simulationDuration", DoubleValue(config["simulationDuration"]));

        // Hand the parsed JSON data to the device builder
        this->fanetDevices->SetTrafficProfiles(this->m_trafficProfiles); 
 
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
    }
}