#include "FANETTopologyHelper.h"
#include "ns3/mobility-module.h"
#include "ns3/FANETMobilityHelper.h"

namespace ns3
{

    NS_LOG_COMPONENT_DEFINE("FANETTopologyHelper");

    FANETTopologyHelper::FANETTopologyHelper(uint32_t nClusters, uint32_t nClusterMems) 
    {
        GDTNode.Create(1);
        allNodes.Add(GDTNode);
    }

    FANETTopologyHelper::~FANETTopologyHelper()
    {
    }

    NodeContainer FANETTopologyHelper::CreateCluster(uint32_t nClusterMems)
    {
        NodeContainer cluster;
        cluster.Create(nClusterMems);
        allNodes.Add(cluster);
        return cluster;
    }

    // void FANETTopologyHelper::StoreClusterMembers(uint32_t nClusters)
    // {
    //     for (uint32_t i = 0; i < nClusters; i++){

    //         clustersCMNodes.push_back(clusters[i]);
    //     }
    // }

    void FANETTopologyHelper::CreateClusters(uint32_t nClusters, std::vector<uint32_t> nClusterNodes)
    {
        for (uint32_t i = 0; i < nClusters; i++)
            clusters.push_back(FANETTopologyHelper::CreateCluster(nClusterNodes[i]));

        //StoreClusterMembers();
    }

    void FANETTopologyHelper::ApplyMobility()
    {
        FANETMobilityHelper mobilityHelper; 
        mobilityHelper.ApplyMobilityWireless(this);
    }

    void FANETTopologyHelper::CreateFANET(uint32_t nClusters, std::vector<uint32_t> nClusterNodes)
    {
        CreateClusters(nClusters, nClusterNodes);
        ApplyMobility();
    }
}