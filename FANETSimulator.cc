#include "FANETSimulator.h"

namespace ns3 
{
    FANETSimulator::FANETSimulator()
    {

    }

    FANETSimulator::~FANETSimulator()
    {

    }

    void FANETSimulator::RunBasicSimulation()
    {
        std::cout << "Number of clusters to simulate: ";
        std::string input;
        std::getline(std::cin, input);  // Read input as a string

        uint32_t nClusterHeads = std::stoul(input);  // Convert string to uint32_t

        std::vector<uint32_t> nClusterNodes;

        for (uint32_t i = 0; i < nClusterHeads; i++)
        {
            std::cout << "Number of cluster nodes in Cluster " << i <<": ";
            std::getline(std::cin, input);  // Read input as a string
            uint32_t tmp = std::stoul(input);
            nClusterNodes.push_back(tmp);
        }

        
        // Print the vector contents properly
        std::cout << "Cluster Nodes: ";
        for (uint32_t nodes : nClusterNodes) {
            std::cout << nodes << " ";
        }
        std::cout << std::endl;
    }
}