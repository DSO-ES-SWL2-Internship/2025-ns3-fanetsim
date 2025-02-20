#ifndef FANET_ANIMATION_HELPER_H
#define FANET_ANIMATION_HELPER_H

#include "ns3/netanim-module.h"
#include "FANETTopologyHelper.h"

#include <cstdint>
#include <string>

namespace ns3 
{
    class FANETAnimationHelper {
        private:
            std::string outputFilename;
            uint64_t maxPkPerFile;
            uint8_t colorIndex;
            std::vector<std::array<uint8_t, 3>> clustersColor;
            std::vector<Ptr<Node>> curCHNodes;
        
            
        public:
            ns3::AnimationInterface* anim;  // Pointer for dynamic allocation

            FANETAnimationHelper(std::string name);
            ~FANETAnimationHelper(); // Destructor

            void SetMaxPktsPerTrFile(uint64_t number);
            void AssignClusterAnim(uint32_t clusterIndex, NodeContainer cluster);
            void AssignGDTAnim(NodeContainer gdtNode);
            void AssignCHAnim(std::vector<Ptr<Node>> CHNodes);
            void UpdateCHAnim(std::vector<Ptr<Node>> CHNodes);
            void AnimateFANET(FANETTopologyHelper& fanet);
    };
}

#endif