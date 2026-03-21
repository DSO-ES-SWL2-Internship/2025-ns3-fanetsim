#include "FANETTopologyHelper.h"
#include "FANETMobilityHelper.h"

namespace ns3
{

    NS_LOG_COMPONENT_DEFINE("FANETTopologyHelper");

    FANETTopologyHelper::FANETTopologyHelper()
    {
        GDTNode.Create(1);
        allNodes.Add(GDTNode);
    }

    FANETTopologyHelper::FANETTopologyHelper(uint32_t nClusters, std::vector<uint32_t> nClusterNodes)
    {
        GDTNode.Create(1);
        allNodes.Add(GDTNode);
    }

    FANETTopologyHelper::~FANETTopologyHelper()
    {
    }

    TypeId FANETTopologyHelper::GetTypeId()
    {
        static TypeId tid =
            TypeId("ns3::FANETTopologyHelper")
                .SetParent<ns3::Object>()
                .AddConstructor<FANETTopologyHelper>();
        
        return tid;
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

        NS_LOG_INFO("Clusters Created");
    }

    void FANETTopologyHelper::CreateFANET(uint32_t nClusters, std::vector<uint32_t> nClusterNodes)
    {
        NS_LOG_INFO("Creating FANET...");
        CreateClusters(nClusters, nClusterNodes);
    }
}