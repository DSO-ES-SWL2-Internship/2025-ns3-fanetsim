#include "FANETMobilityHelper.h"
#include "ns3/core-module.h"

namespace ns3
{

    NS_LOG_COMPONENT_DEFINE("FANETMobilityHelper");

    // Constructor
    FANETMobilityHelper::FANETMobilityHelper() {}

    // Destructor
    FANETMobilityHelper::~FANETMobilityHelper() {}

    // Set the GDT node at a fixed position
    void FANETMobilityHelper::SetGDTMobility(NodeContainer& gdtNode) {

        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
        positionAlloc->Add(Vector(0.0, 0.0, 0.0)); // Static GDT Node
        mobility.SetPositionAllocator(positionAlloc);

        // mobility.SetPositionAllocator("ns3::ListPositionAllocator",
        //                             "Positions", VectorValue({Vector(0.0, 0.0, 0.0)}));  // GDT at (0,0)
        
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(gdtNode);

        NS_LOG_INFO("GDT mobility set");
    }


    // // Set fixed positions for cluster heads in a circular layout around GDT
    // void FANETMobilityHelper::SetClusterHeadMobility(NodeContainer& clusterHeads, double x, double y, uint32_t nClusterHeads, double radius) {
    //     Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    //     // Calculate positions for each cluster head in a circular layout
    //     for (uint32_t i = 0; i < nClusterHeads; ++i) {
    //         double angle = 2 * M_PI * i / nClusterHeads; // Evenly distribute cluster heads
    //         double clusterX = x + radius * cos(angle);   // X position based on angle
    //         double clusterY = y + radius * sin(angle);   // Y position based on angle

    //         positionAlloc->Add(Vector(clusterX, clusterY, 0.0)); // Set position
    //     }

    //     // Assign the position allocator to the mobility model
    //     mobility.SetPositionAllocator(positionAlloc);
    //     mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel"); // Fixed position
    //     mobility.Install(clusterHeads);
    // }


    // Set random mobility for cluster members within a sector around their CH
    void FANETMobilityHelper::SetClusterMemberMobility(NodeContainer& clusterMembers, double xCenter, double yCenter) {
            // mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", 
            // "Bounds", RectangleValue(Rectangle(-100, 100, -100, 100)), // Increase bounds
            // "Speed", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"));
            // mobility.Install(fanet.clusters[1]);
        mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator", 
            "X", StringValue("ns3::UniformRandomVariable[Min=" + std::to_string(xCenter - 10.0) + "|Max=" + std::to_string(xCenter + 10.0) + "]"),
            "Y", StringValue("ns3::UniformRandomVariable[Min=" + std::to_string(yCenter - 10.0) + "|Max=" + std::to_string(yCenter + 10.0) + "]"));

        // Set the Mobility Model with larger bounds for Random Walk
        mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", 
            "Bounds", RectangleValue(Rectangle(xCenter - 100.0, xCenter + 100.0, yCenter - 100.0, yCenter + 100.0)),
            "Speed", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=50.0]"));


        mobility.Install(clusterMembers);
    }

    // // Apply mobility to all nodes in the FANET topology, cluster heads are set to be not mobile in this model as P2P is used
    // void FANETMobilityHelper::ApplyMobilityP2P(FANETTopologyHelper* fanet) {
    //     NodeContainer singleCH;
    //     SetGDTMobility(fanet->GDTNode);

    //     double radius = 100.0;  // Distance of cluster heads from GDT
    //     double angleStep = 360.0 / fanet->clusters.size();  // Evenly space CHs in a circular pattern

    //     for (size_t i = 0; i < fanet->clusters.size(); i++) {
    //         double angleRad = (angleStep * i) * (M_PI / 180.0);
    //         double xCH = radius * cos(angleRad);
    //         double yCH = radius * sin(angleRad);
            
    //         // Set mobility for members of this cluster around their CH
    //         SetClusterMemberMobility(fanet->clustersCMNodes[i], xCH, yCH);
    //         //SetClusterMemberMobility(fanet->clusters[i], xCH, yCH);
    //     }

    //     SetClusterHeadMobility(fanet->clusterHeadNodes, 0.0, 0.0, fanet->clusters.size(), radius);
    // }

    void FANETMobilityHelper::ApplyMobilityWireless(FANETTopologyHelper* fanet) {
        NodeContainer singleCH;
        SetGDTMobility(fanet->GDTNode);

        double radius = 100.0;  // Distance of cluster heads from GDT
        double angleStep = 360.0 / fanet->clusters.size();  // Evenly space CHs in a circular pattern

        for (size_t i = 0; i < fanet->clusters.size(); i++) {
            double angleRad = (angleStep * i) * (M_PI / 180.0);
            double xCH = radius * cos(angleRad);
            double yCH = radius * sin(angleRad);
            
            // Set mobility for members of this cluster to be around the area of their assignment
            SetClusterMemberMobility(fanet->clusters[i], xCH, yCH);
        }

        NS_LOG_INFO("Cluster Nodes mobility set");
    }


    Ptr<Node> FANETMobilityHelper::GetClosestNode(Ptr<Node> target, NodeContainer nodes)
    {
        Ptr<MobilityModel> targetMobility = target->GetObject<MobilityModel>();

        double minDistance = std::numeric_limits<double>::max();
        Ptr<Node> closestNode = nullptr;

        // Iterate through all nodes in the cluster to find the closest one
        for (uint32_t j = 0; j < nodes.GetN(); j++)
        {
            Ptr<Node> node = nodes.Get(j);
            Ptr<MobilityModel> nodeMobility = node->GetObject<MobilityModel>();

            if (nodeMobility && targetMobility)
            {
                double distance = targetMobility->GetDistanceFrom(nodeMobility);

                if (distance < minDistance)
                {
                    minDistance = distance;
                    closestNode = node;
                }
            }
        }

        return closestNode;
    }

}