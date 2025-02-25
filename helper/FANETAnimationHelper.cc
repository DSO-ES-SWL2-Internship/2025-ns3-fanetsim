#include "FANETAnimationHelper.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("FANETAnimationHelper");

    FANETAnimationHelper::FANETAnimationHelper(std::string name)
        : AnimationInterface(name)
    {
    }

    FANETAnimationHelper::~FANETAnimationHelper()
    {
    }


    void FANETAnimationHelper::AssignClusterAnim(uint32_t clusterIndex, NodeContainer cluster) {
        uint8_t r = (clusterIndex * 50) % 255;
        uint8_t g = (clusterIndex * 80) % 255;
        uint8_t b = (clusterIndex * 120) % 255;

        for (uint32_t i = 0; i < cluster.GetN(); i++) {
            // e.g. C1Node-3_1 - Node 3 of cluster 1 with node index 1
            UpdateNodeDescription(cluster.Get(i), "C" + std::to_string(clusterIndex) + "Node-" + std::to_string(i) + "_" + std::to_string(cluster.Get(i)->GetId()));
            UpdateNodeColor(cluster.Get(i), r, g, b);
        }

        clustersColor.push_back({r,g,b});
    }

    void FANETAnimationHelper::AssignGDTAnim(NodeContainer GDTNode){
        UpdateNodeColor(GDTNode.Get(0), 255, 0, 0);
    }

    void FANETAnimationHelper::AssignCHAnim(std::vector<Ptr<Node>> CHNodes)
    {
        for (size_t i = 0; i < CHNodes.size(); i++)
        {
            UpdateNodeColor(CHNodes[i], 236, 3, 252);
            curCHNodes.push_back(CHNodes[i]);
        }
    }

    void FANETAnimationHelper::UpdateCHAnim(std::vector<Ptr<Node>> CHNodes)
    {
        //NS_LOG_UNCOND("ENtered");
        for (size_t i = 0; i < CHNodes.size(); i++)
        {
            if (curCHNodes[i]->GetId() != CHNodes[i]->GetId())
            {
                // Update new CH to CH color
                UpdateNodeColor(CHNodes[i], 236, 3, 252);
                // Update old CH to cluster color
                UpdateNodeColor(curCHNodes[i], clustersColor[i][0], clustersColor[i][1], clustersColor[i][2]);

                curCHNodes[i] = CHNodes[i];
            }
        }
    }

    void FANETAnimationHelper::AnimateFANET(FANETTopologyHelper& fanet) {

        for (size_t i = 0; i < fanet.clusters.size(); i++) {
            AssignClusterAnim(i, fanet.clusters[i]);
        }

        AssignGDTAnim(fanet.GDTNode);

        AssignCHAnim(fanet.CHNodes);

        EnablePacketMetadata(true);
    }
}