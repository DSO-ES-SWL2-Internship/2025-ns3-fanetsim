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
        //Loop to find cluster nodes and print their interfaces and SSIDs
        for (size_t i = 0; i < this->fanet->clusters.size(); i++) {
            for (uint32_t j = 0; j < this->fanet->clusters[i].GetN(); j++) 
            {
                Ptr<Node> clusterNode = this->fanet->clusters[i].Get(j);
                std::cout << "Node " << clusterNode->GetId() << " has " << clusterNode->GetNDevices() << " interfaces. ";
        
                for (uint32_t d = 0; d < clusterNode->GetNDevices(); d++) {
                    Ptr<NetDevice> genericDevice = clusterNode->GetDevice(d);
                    Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(genericDevice);
    
                    if (wifiDev) {
                        std::cout << "  -> Device " << d << " is Wi-Fi. SSID: " << wifiDev->GetMac()->GetSsid() << std::endl;
                    } else {
                        std::cout << "  -> Device " << d << " is NOT Wi-Fi (Likely Loopback)." << std::endl;
                    }
        }
        std::cout << std::endl;
            }
        }
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
        LogComponentEnable("UdpSocketImpl", LOG_LEVEL_INFO); 
        
        // See if AODV is desperately crying out for a route but failing
        //LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_LOGIC); 
        
        //See if the TDMA MAC layer is properly prioritizing and scheduling packets according to the traffic profiles
        LogComponentEnable("TdmaWifiMac", LOG_LEVEL_FUNCTION);

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
        //try to schedule the cluster head assignment a little later to ensure all the routing tables are populated and 
        //the GDT is fully aware of the cluster nodes before it tries to assign them as cluster heads.
        Simulator::Schedule(Seconds(0.001), &FANETDeviceHelper::AssignClusterHeads, this->fanetDevices, this->fanet, this->ipv4, this->anim);

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
        //For simplicity, we'll just set up three types of traffic: Video, Status, and Command, each with different bandwidth requirements and priorities. 
        //The TDMA MAC layer will use the priorities to allocate mini-slots accordingly.
        //VIDEO: 1.2K High Res -> 1200 bytes, 9.6Kbps | Priority 3 (DSCP 0x60)
        OnOffHelper videoApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        videoApp.SetConstantRate(DataRate("9.6Kbps"), 1200); 
        videoApp.SetAttribute("Tos", UintegerValue(0x60)); 

        //STATUS 1: 0.1K -> 100 bytes, 0.8Kbps | Priority 2 (DSCP 0x80)
        OnOffHelper statusApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        statusApp.SetConstantRate(DataRate("0.8Kbps"), 100); 
        statusApp.SetAttribute("Tos", UintegerValue(0x80)); 

        //CMD 1: 0.1K -> 100 bytes, 0.8Kbps | Priority 3 (DSCP 0x60)
        OnOffHelper cmdApp("ns3::UdpSocketFactory", InetSocketAddress(gcsIp, port));
        cmdApp.SetConstantRate(DataRate("0.8Kbps"), 100); 
        cmdApp.SetAttribute("Tos", UintegerValue(0x60)); 

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

        this->anim->AnimateFANET(this->fanet);

        //Set up a mechanism for the GCS to send a dynamic command to a target node at runtime, 
        //demonstrating the ability to interact with the network after it's already up and running.
        //Force Node 1 to listen on port 9999
        Ptr<Node> targetNode= this->fanet->clusters[0].Get(0); 
        Ptr<Socket> cmdSocket = Socket::CreateSocket(targetNode, UdpSocketFactory::GetTypeId());
        cmdSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 9999));
        cmdSocket->SetRecvCallback(MakeCallback(&FANETSimulator::DynamicCommandRxCallback, this));

        //Get the IP address of that target node so the GCS knows where to aim
        Ptr<Ipv4> ipv4 = targetNode->GetObject<Ipv4>();
        Ipv4Address targetIp = ipv4->GetAddress(1, 0).GetLocal();

        //Schedule the GCS to send a command to that node after 15 seconds of simulation time, 
        //which should be well after the network is established and the applications are actively sending data.
        Simulator::Schedule(Seconds(15.0), &FANETSimulator::SendDynamicCommand, this, gcsNode, targetIp);

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


    //Dynamic GCS command and MAC layer reallocation callback implementations
    void FANETSimulator::SendDynamicCommand(Ptr<Node> gcsNode, Ipv4Address targetNodeIp)
    {
        std::cout << "\n" << std::endl;
        std::cout << "[GCS COMMAND] Time: " << Simulator::Now().As(Time::S) << std::endl;
        std::cout << "[GCS COMMAND] Transmitting 'HIGH_RES' command to Node: " << targetNodeIp << std::endl;
        std::cout << "\n" << std::endl;

        //Create a UDP Socket on the GCS
        Ptr<Socket> socket = Socket::CreateSocket(gcsNode, UdpSocketFactory::GetTypeId());
        
        //onnect to the Node's special command port (Port 9999)
        socket->Connect(InetSocketAddress(targetNodeIp, 9999));
        
        //Fire the packet with the command. In a real scenario, this could be a more complex packet with specific headers, but for simplicity, we're just sending a plain packet with "HIGH_RES" as its content.
        Ptr<Packet> packet = Create<Packet>((uint8_t*)"HIGH_RES", 8);
        socket->Send(packet);
    }

    void FANETSimulator::DynamicCommandRxCallback(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        while ((packet = socket->Recv()))
        {
            //The Node received the packet
            Ptr<Node> rxNode = socket->GetNode();
            std::cout << "\n[Node Listener] Time: " << Simulator::Now().As(Time::S) << std::endl;
            std::cout << "[Node Listener] Node " << rxNode->GetId() << " received command! Re-allocating TDMA Slots..." << std::endl;

            //Define the new, heavier Traffic Profile (Video jumps from Priority 3/4 to Priority 5 with 2.4Kbps)
            std::vector<TrafficProfile> newHighResProfiles = {
                {"Cmd3", 1, 0.6},
                {"Status2", 1, 0.3},
                {"Video_HIGH_RES", 5, 2.4} // <-- Requires more mini-slots!
            };

            //Loop through the Node's hardware to find the TdmaWifiMac chip
            for (uint32_t i = 0; i < rxNode ->GetNDevices(); i++) {
                Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(rxNode->GetDevice(i));
                if (wifiDev) {
                    Ptr<TdmaWifiMac> tdmaMac = DynamicCast<TdmaWifiMac>(wifiDev->GetMac());
                    if (tdmaMac) {
                        //Inject the new profile. 
                        //This will instantly trigger AllocateMiniSlots() and print the new table!
                        tdmaMac->SetTrafficProfiles(newHighResProfiles);
                    }
                }
            }
        }
    }
}   