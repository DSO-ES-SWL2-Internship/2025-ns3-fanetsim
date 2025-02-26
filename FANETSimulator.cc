#include "FANETSimulator.h"
#include "ns3/applications-module.h"
#include "ns3/add-client.h"
#include "ns3/add-server.h"

namespace ns3 
{

    NS_LOG_COMPONENT_DEFINE("FANETSimulator");

    FANETSimulator::FANETSimulator()
    {

    }

    FANETSimulator::~FANETSimulator()
    {
        delete this->mobility;
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

    void FANETSimulator::CreateNetwork()
    {
        this->fanet = new FANETTopologyHelper(this->nClusters, this->nClusterNodes);
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
        this->fanetDevices = new FANETDeviceHelper();
        this->fanetDevices->TdmaWifi();
        this->fanetDevices->SetupClustersWifi(this->fanet->clusters);
        this->fanetDevices->SetUpLinksWifi(this->fanet);
        this->fanetDevices->AssignTdmaSlots(this->fanet->allNodes, MilliSeconds(this->cycleDuration));
    }

    void FANETSimulator::SetRoutingProtocol(RoutingProtocol protocol)
    {
        this->router = new FANETRoutingHelper();
        switch (protocol)
        {
            case AODV: {
                // TODO: add functionality to include all the different settings for AODV
                this->router->SetAODV(this->fanet->allNodes);
                break;
            }

            case OLSR: {
                this->router->SetOLSR(this->fanet->allNodes);
                break;
            }

            case DSDV: {
                this->router->SetDSDV(this->fanet->allNodes);
                break;
            }

            default: {
                NS_LOG_UNCOND("Invalid Routing Protocol");
                exit(FAILURE);
            }
        }
    }

    void FANETSimulator::AssignAddress(Ipv4Address network, Ipv4Mask mask)
    {
        this->ipv4 = new FANETAddressHelper(network, mask);
        this->ipv4->SetBases(&(this->gdtVirtualAddress), this->fanetDevices->clustersDevices, this->fanetDevices->clustersLinkDevices);
    }

    void FANETSimulator::SetUpNetAnim()
    {
        this->anim = new FANETAnimationHelper(this->fileName);
        anim->SetMaxPktsPerTraceFile(5000000);
    }


    void FANETSimulator::RunBasicSimulation(std::string fileName)
    {
        NS_LOG_INFO("Setting XML output file to: " << this->fileName);
        this->fileName = fileName;

        this->GetNClusters();

        this->GetNClusterNodes();

        this->GetCycleDuration();

        this->CreateNetwork();

        this->SetMobility();

        this->InstallDevices();

        this->SetRoutingProtocol(AODV);

        this->AssignAddress("10.1.1.0", "255.255.255.0");

        this->SetUpNetAnim();

        this->fanetDevices->AssignClusterHeads(this->fanet, this->ipv4, this->anim);

        // Install UDP Echo Server on a cluster 1 node
        UdpEchoServerHelper echoServer(9);
        ApplicationContainer serverApp = echoServer.Install(fanet->clusters[1].Get(1));
        serverApp.Start(Seconds(2.0));
        serverApp.Stop(Seconds(20.0));

        // Install UDP Echo Client on last node
        
        UdpEchoClientHelper echoClient(this->ipv4->clustersInterfaces[1].GetAddress(1), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(5));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(5)));
        echoClient.SetAttribute("PacketSize", UintegerValue(512));

        ApplicationContainer clientApp = echoClient.Install(this->fanet->clusters[0].Get(1));
        clientApp.Start(Seconds(3.0));
        clientApp.Stop(Seconds(20.0));

        // Ptr<AddServer> serverApp = CreateObject<AddServer>();
        // fanet->clusters[0].Get(1)->AddApplication(serverApp);
        // serverApp->SetStartTime(Seconds(2.0));
        // serverApp->SetStopTime(Seconds(20.0));

        // Ptr<AddClient> clientApp = CreateObject<AddClient>();
        // clientApp->Setup(this->ipv4->clustersInterfaces[0].GetAddress(1), 8080);


        // for (uint32_t i = 1; i < fanet->allNodes.GetN(); ++i) {
        //     Ptr<AddClient> senderApp = CreateObject<AddClient>();
        //     senderApp->Setup(, 8080);
        //     nodes.Get(i)->AddApplication(senderApp);
        //     senderApp->SetStartTime(Seconds(2.0));
        //     senderApp->SetStopTime(Seconds(20.0));
        // }


        this->anim->AnimateFANET(this->fanet);

        Simulator::Stop(Seconds(20.0));
        Simulator::Run();
        Simulator::Destroy();

    }
}