#include "FANETTopologyHelper.h"

namespace ns3
{
    FANETTopologyHelper::FANETTopologyHelper() 
        : nClusterHeads(0), nClusterMems(0)
    {
        GDTNode.Create(1);
        allNodes.Add(GDTNode);
    }

    FANETTopologyHelper::FANETTopologyHelper(uint32_t nClusterHeads, uint32_t nClusterMems) 
        : nClusterHeads(nClusterHeads), nClusterMems(nClusterMems)
    {
        GDTNode.Create(1);
        allNodes.Add(GDTNode);
    }

    FANETTopologyHelper::~FANETTopologyHelper()
    {
    }

    NodeContainer FANETTopologyHelper::CreateCluster()
    {
        NodeContainer cluster;
        cluster.Create(nClusterMems);
        allNodes.Add(cluster);
        return cluster;
    }

    void FANETTopologyHelper::StoreClusterMembers()
    {
        for (uint32_t i = 0; i < nClusterHeads; i++){
            NodeContainer clusterMems;
            if (nClusterMems > 1){
                for (uint32_t j = 1; j < nClusterMems; j++){
                clusterMems.Add(clusters[i].Get(j));
            }
            }

            clustersCMNodes.push_back(clusterMems);
        }
    }

    void FANETTopologyHelper::CreateClusters()
    {
        for (uint32_t i = 0; i < nClusterHeads; i++)
            clusters.push_back(FANETTopologyHelper::CreateCluster());

        StoreClusterMembers();
    }

    void FANETTopologyHelper::CreateLinks()
    {
        for (uint32_t i = 0; i < nClusterHeads; i++){
            NodeContainer link;
            link.Add(GDTNode.Get(0), clusters[i].Get(0));
            clusterHeadNodes.Add(clusters[i].Get(0));
            GDTtoCHLinkNodes.push_back(link);
        }
    }

    void FANETTopologyHelper::SetupFANET(uint32_t nClusterHeads, uint32_t nClusterMembers)
    {
        this->nClusterHeads = nClusterHeads;
        this->nClusterMems = nClusterMembers;
    }

    void FANETTopologyHelper::CreateFANET()
    {
        CreateClusters();
        CreateLinks();
    }

}