#include "FANETTopologyHelper.h"
#include "ns3/mobility-module.h"
#include "ns3/FANETMobilityHelper.h"

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

    void FANETTopologyHelper::CreateP2PLinks()
    {
        FANETMobilityHelper mobilityHelper;
        mobilityHelper.ApplyMobilityP2P(this);

        for (uint32_t i = 0; i < nClusterHeads; i++){
            NodeContainer link;
            link.Add(GDTNode.Get(0), clusters[i].Get(0));
            clusterHeadNodes.Add(clusters[i].Get(0));
            GDTtoCHLinkNodes.push_back(link);
        }
    }

    void FANETTopologyHelper::CreateWirelessLinksv1()
    {
        FANETMobilityHelper mobilityHelper;
        mobilityHelper.ApplyMobilityWireless(this);

        for (uint32_t i = 0; i < nClusterHeads; i++){
            NodeContainer link;
            link.Add(GDTNode.Get(0), clusters[i].Get(0));
            clusterHeadNodes.Add(clusters[i].Get(0));
            GDTtoCHLinkNodes.push_back(link);
            NS_LOG_UNCOND("Node " << clusters[i].Get(0)->GetId() << " selected as cluster head");
        }
    }

    void FANETTopologyHelper::CreateWirelessLinksv2()
    {
        FANETMobilityHelper mobilityHelper;
        mobilityHelper.ApplyMobilityWireless(this);

        Ptr<Node> gdt = GDTNode.Get(0);
        Ptr<MobilityModel> gdtMobility = gdt->GetObject<MobilityModel>();

        for (uint32_t i = 0; i < nClusterHeads; i++)
        {
            double minDistance = std::numeric_limits<double>::max();
            Ptr<Node> selectedCH = nullptr;

            // Iterate through all nodes in the cluster to find the closest one
            for (uint32_t j = 0; j < clusters[i].GetN(); j++)
            {
                Ptr<Node> clusterNode = clusters[i].Get(j);
                Ptr<MobilityModel> nodeMobility = clusterNode->GetObject<MobilityModel>();

                if (nodeMobility && gdtMobility)
                {
                    double distance = gdtMobility->GetDistanceFrom(nodeMobility);

                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        selectedCH = clusterNode;
                    }
                }
            }

            if (selectedCH)
            {
                NodeContainer link;
                link.Add(gdt, selectedCH);
                clusterHeadNodes.Add(selectedCH);
                GDTtoCHLinkNodes.push_back(link);
                NS_LOG_UNCOND("Node " << selectedCH->GetId() << " selected as cluster head");
            }
        }
    }

    void FANETTopologyHelper::SetupFANET(uint32_t nClusterHeads, uint32_t nClusterMembers)
    {
        this->nClusterHeads = nClusterHeads;
        this->nClusterMems = nClusterMembers;
    }

    void FANETTopologyHelper::CreateFANETP2P()
    {
        CreateClusters();
        CreateP2PLinks();
    }

    // Create a wireless FANET where the first node of each cluster is the cluster head
    void FANETTopologyHelper::CreateFANETWirelessv1()
    {
        CreateClusters();
        CreateWirelessLinksv1();
    }

    // Create a FANET where the closest node of each cluster is selected to be the cluster head
    void FANETTopologyHelper::CreateFANETWirelessv2()
    {
        CreateClusters();
        CreateWirelessLinksv2();
    }

}