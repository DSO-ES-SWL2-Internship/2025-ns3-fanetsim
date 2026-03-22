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
        this->fanetDevices->SetupGDTWifi(this->fanet->GDTNode);
        this->fanetDevices->SetupClustersWifi(this->fanet->clusters);
        this->fanetDevices->SetUpLinksWifi(this->fanet);
        this->fanetDevices->AssignTdmaSlots(this->fanet->allNodes, MilliSeconds(this->cycleDuration));
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

        // Setup application on all the nodes
        InstallApplication<GDTApp>(
            this->fanet->GDTNode.Get(0), 0.0, simDuration,
            [this](Ptr<GDTApp> app) {
                app->SetPort(8080);
                app->EnableInfoLog();
                app->m_plrManager->Setup(this->fanet->allNodes.GetN());
                
            }          
        );
        
        for (size_t i = 0; i < this->fanet->clusters.size(); i++)
        {
            for (uint32_t j = 0; j < this->fanet->clusters[i].GetN(); j++)
            {
                InstallApplication<ClusterNodeApp>(
                    this->fanet->clusters[i].Get(j), 0.0, simDuration,
                    [this, i](Ptr<ClusterNodeApp> app) {
                        app->SetUp(this->ipv4->GDTInterface.GetAddress(0), 8080, i, this->ipv4->GetClusterBaseIP(i));
                        //app->SetupPLR(this->fanet->allNodes.GetN());
                        app->m_plrManager->Setup(this->fanet->allNodes.GetN());   
                        app->EnableInfoLog();
                    }
                ); 
            }
        }

        // Starting PLR test on some nodes. Note: as of now if there is no mechanism to check what if the response packet from the query was not received.
        Ptr<Node> node = this->fanet->GDTNode.Get(0);
        Ptr<FANETApplication> gdtapp = DynamicCast<FANETApplication>(node->GetApplication(0));
        if (gdtapp) gdtapp->m_plrManager->StartTest(gdtapp, 3, 0, NETWORK_BROADCAST, 20, 0.5);

        auto& selNode = *this->fanet->clusters[0].Get(0);
        Ptr<FANETApplication> app = DynamicCast<FANETApplication>(node->GetApplication(0));
        if (app) app->m_plrManager->StartTest(app, 10, selNode.GetId(), NETWORK_BROADCAST, 20, 0.5);

        // node = this->fanet->clusters[1].Get(2);
        // Ptr<ClusterNodeApp> app = DynamicCast<ClusterNodeApp>

        LogComponentDisable("ClusterNodeCHPromo", LOG_LEVEL_DEBUG);

        this->SetUpNetAnim();

        this->fanetDevices->AssignClusterHeads(this->fanet, this->ipv4, this->anim);

        // Install UDP Echo Server on a cluster 1 node
        // UdpEchoServerHelper echoServer(9);
        // ApplicationContainer serverApp = echoServer.Install(this->fanet->GDTNode.Get(0));
        // serverApp.Start(Seconds(2.0));
        // serverApp.Stop(Seconds(20.0));

        // Install UDP Echo Client on last node
        
        // UdpEchoClientHelper echoClient1(this->ipv4->GDTInterface.GetAddress(0), 9);
        // echoClient1.SetAttribute("MaxPackets", UintegerValue(5));
        // echoClient1.SetAttribute("Interval", TimeValue(Seconds(5)));
        // echoClient1.SetAttribute("PacketSize", UintegerValue(512));

        // ApplicationContainer clientApp1 = echoClient1.Install(this->fanet->clusters[0].Get(1));
        // clientApp1.Start(Seconds(3.0));
        // clientApp1.Stop(Seconds(20.0));

        // UdpEchoClientHelper echoClient2(this->ipv4->GDTInterface.GetAddress(0), 9);
        // echoClient2.SetAttribute("MaxPackets", UintegerValue(1));
        // echoClient2.SetAttribute("Interval", TimeValue(Seconds(5)));
        // echoClient2.SetAttribute("PacketSize", UintegerValue(512));

        // ApplicationContainer clientApp2 = echoClient2.Install(this->fanet->clusters[2].Get(2));
        // clientApp2.Start(Seconds(3.0));
        // clientApp2.Stop(Seconds(20.0));

        // Ptr<AddServer> serverApp = CreateObject<AddServer>();
        // this->fanet->GDTNode.Get(0)->AddApplication(serverApp);
        // serverApp->SetStartTime(Seconds(0.0));
        // serverApp->SetStopTime(Seconds(20.0));

        // Ptr<AddClient> clientApp1 = CreateObject<AddClient>();
        // clientApp1->Setup(this->ipv4->GDTInterface.GetAddress(0), 8080);
        // this->fanet->clusters[1].Get(2)->AddApplication(clientApp1);
        // clientApp1->SetStartTime(Seconds(3.0));
        // clientApp1->SetStopTime(Seconds(20.0));

        // Ptr<AddClient> clientApp2 = CreateObject<AddClient>();
        // clientApp2->Setup(this->ipv4->GDTInterface.GetAddress(0), 8080);
        // this->fanet->clusters[2].Get(2)->AddApplication(clientApp2);
        // clientApp2->SetStartTime(Seconds(3.0));
        // clientApp2->SetStopTime(Seconds(20.0));


        // for (uint32_t i = 1; i < fanet->allNodes.GetN(); ++i) {
        //     Ptr<AddClient> senderApp = CreateObject<AddClient>();
        //     senderApp->Setup(, 8080);
        //     nodes.Get(i)->AddApplication(senderApp);
        //     senderApp->SetStartTime(Seconds(2.0));
        //     senderApp->SetStopTime(Seconds(20.0));
        // }


        this->anim->AnimateFANET(this->fanet);

        Simulator::Stop(Seconds(simDuration));
        Simulator::Run();
        Simulator::Destroy();
    }

    void FANETSimulator::SetupSimulation(std::string jsonFilePath)
    {
        std::ifstream file(jsonFilePath);
        json config;
        file >> config;

        Setup();
        SetAttribute("nClusters", UintegerValue(config["nClusters"]));
        SetAttribute("nClusterNodes", StringValue(config["nClusterNodes"].get<std::string>()));
        ParseClusterNodesString();
        SetAttribute("cycleDuration", UintegerValue(config["cycleDuration"]));
        SetAttribute("filename", StringValue(config["filename"].get<std::string>()));
        SetAttribute("simulationDuration", DoubleValue(config["simulationDuration"]));
 
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