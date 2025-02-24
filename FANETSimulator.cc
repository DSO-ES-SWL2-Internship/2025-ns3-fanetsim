#include "FANETSimulator.h"

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

    void FANETSimulator::RunBasicSimulation(std::string fileName)
    {
        NS_LOG_INFO("Setting XML output file to: " << this->fileName);
        this->fileName = fileName;

        this->GetNClusters();

        this->GetNClusterNodes();

        this->CreateNetwork();

        this->SetMobility();

    }
}