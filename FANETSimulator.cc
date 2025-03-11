#include "FANETSimulator.h"
#include "ns3/applications-module.h"
#include "ns3/add-client.h"
#include "ns3/add-server.h"
#include "ns3/cluster-node-app.h"
#include "ns3/gdt-app.h"
#include "ns3/FANETAppHelper.h"

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
        this->fanetDevices->SetupGDTWifi(this->fanet->GDTNode);
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
        this->ipv4->SetBases(this->fanetDevices->GDTDevice,  this->fanetDevices->clustersDevices, this->fanetDevices->clustersLinkDevices);
    }

    void FANETSimulator::SetUpNetAnim()
    {
        this->anim = new FANETAnimationHelper(this->fileName);
        anim->SetMaxPktsPerTraceFile(5000000);
    }


    void FANETSimulator::RunSimulation(std::string fileName)
    {
        NS_LOG_INFO("Setting XML output file to: " << this->fileName);
        this->fileName = fileName;

        

        ns3::PacketMetadata::Enable();

        this->GetNClusters();

        this->GetNClusterNodes();

        this->GetCycleDuration();

        this->GetSimulationDuration();

        this->CreateNetwork();

        this->SetMobility();

        this->InstallDevices();

        this->SetRoutingProtocol(AODV);

        this->AssignAddress("10.1.1.0", "255.255.255.0");


        InstallApplication<GDTApp>(
            this->fanet->GDTNode.Get(0), 0.0, simDuration,
            [](Ptr<GDTApp> app) {app->SetPort(8080);},
            [](Ptr<GDTApp> app) {app->EnableInfoLog();}
        );
        
        for (size_t i = 0; i < this->fanet->clusters.size(); i++)
        {
            for (uint32_t j = 0; j < this->fanet->clusters[i].GetN(); j++)
            {
                if (i == 0 && j == 2)
                {
                    InstallApplication<ClusterNodeApp>(
                        this->fanet->clusters[i].Get(j), 0.0, simDuration,
                        [this, i](Ptr<ClusterNodeApp> app) {

                            app->SetUp(this->ipv4->GDTInterface.GetAddress(0), 8080, i);
                            
                            app->m_plrManager->Setup(this->fanet->allNodes.GetN());     
                            app->m_plrManager->StartPLRp2p(app, 0.5, this->fanet->clusters[1].Get(2)->GetId(), 
                                this->ipv4->clustersInterfaces[1].GetAddress(2), 20, 0.5);      
                            app->m_plrManager->StartPLRp2p(app, 0.5, this->fanet->clusters[0].Get(1)->GetId(), 
                                this->ipv4->clustersInterfaces[0].GetAddress(1), 20, 0.5);       
                            app->EnableInfoLog();
                        }
                    );                    
                } 
                else 
                {
                    InstallApplication<ClusterNodeApp>(
                        this->fanet->clusters[i].Get(j), 0.0, simDuration,
                        [this, i](Ptr<ClusterNodeApp> app) {
                            app->SetUp(this->ipv4->GDTInterface.GetAddress(0), 8080, i);
                            //app->SetupPLR(this->fanet->allNodes.GetN());
                            app->m_plrManager->Setup(this->fanet->allNodes.GetN());   
                            app->EnableInfoLog();
                        }
                    );
                }
            }
        }

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
}