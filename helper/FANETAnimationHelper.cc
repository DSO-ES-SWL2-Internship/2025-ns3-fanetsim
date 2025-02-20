#include "FANETAnimationHelper.h"

namespace ns3
{
    FANETAnimationHelper::FANETAnimationHelper(std::string name){
        anim = new AnimationInterface(name);
    }

    FANETAnimationHelper::~FANETAnimationHelper(){
        delete this->anim;
    }

    void FANETAnimationHelper::SetMaxPktsPerTrFile(uint64_t number) {
        maxPkPerFile = number;
    }


    void FANETAnimationHelper::AssignClusterAnim(uint32_t clusterIndex, NodeContainer cluster) {
        uint8_t r = (clusterIndex * 50) % 255;
        uint8_t g = (clusterIndex * 80) % 255;
        uint8_t b = (clusterIndex * 120) % 255;

        for (uint32_t i = 0; i < cluster.GetN(); i++) {
            // e.g. C1Node-3_1 - Node 3 of cluster 1 with node index 1
            anim->UpdateNodeDescription(cluster.Get(i), "C" + std::to_string(clusterIndex) + "Node-" + std::to_string(i) + "_" + std::to_string(cluster.Get(i)->GetId()));
            anim->UpdateNodeColor(cluster.Get(i), r, g, b);
            clustersColor.push_back({r,g,b});
        }
    }

    void FANETAnimationHelper::AssignGDTAnim(NodeContainer GDTNode){
        anim->UpdateNodeColor(GDTNode.Get(0), 255, 0, 0);
    }

    void FANETAnimationHelper::AssignCHAnim(std::vector<Ptr<Node>> CHNodes)
    {
        for (size_t i = 0; i < CHNodes.size(); i++)
        {
            anim->UpdateNodeColor(CHNodes[i], 236, 3, 252);
            curCHNodes.push_back(CHNodes[i]);
        }
    }

    void FANETAnimationHelper::UpdateCHAnim(std::vector<Ptr<Node>> CHNodes)
    {
        for (size_t i = 0; i < CHNodes.size(); i++)
        {
            if (curCHNodes[i]->GetId() != CHNodes[i]->GetId())
            {
                // Update new CH to CH color
                anim->UpdateNodeColor(CHNodes[i], 236, 3, 252);
                // Update old CH to cluster color
                anim->UpdateNodeColor(curCHNodes[i], clustersColor[i][0], clustersColor[i][1], clustersColor[i][2]);

                curCHNodes[i] = CHNodes[i];
            }
        }

        Simulator::Schedule(Seconds(1.0), &FANETAnimationHelper::UpdateCHAnim, this, CHNodes);
    }

    void FANETAnimationHelper::AnimateFANET(FANETTopologyHelper& fanet) {
        if (!anim) {
            anim = new AnimationInterface(outputFilename);  // Ensure it is initialized
        }

        NS_LOG_UNCOND("Set Name");

        anim->SetMaxPktsPerTraceFile(maxPkPerFile);

        for (size_t i = 0; i < fanet.clusters.size(); i++) {
            AssignClusterAnim(i, fanet.clusters[i]);
        }

        NS_LOG_UNCOND("Assigning Clusters");

        AssignGDTAnim(fanet.GDTNode);

        NS_LOG_UNCOND("Assigning GDT");

        AssignCHAnim(fanet.CHNodes);

        anim->EnablePacketMetadata(true);
        //anim->EnableIpv4RouteTracking("udp-routing.xml", Seconds(0), Seconds(20), Seconds(0.25));

        Simulator::Schedule(Seconds(1.0), &FANETAnimationHelper::UpdateCHAnim, this, fanet.CHNodes);
    }
}