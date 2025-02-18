#include "FANETRoutingHelper.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsr-module.h"

namespace ns3
{
    FANETRoutingHelper::FANETRoutingHelper(){

    }

    FANETRoutingHelper::~FANETRoutingHelper(){

    }

    void FANETRoutingHelper::InstallInternetStackToAllNodes(NodeContainer nodes){
        this->internet.SetRoutingHelper(this->list);
        this->internet.Install(nodes);
    }

    void FANETRoutingHelper::SetAODV(NodeContainer nodes){
        AodvHelper aodv;

        // Adjust aodv parameters here

        this->list.Add (aodv, 100);

        InstallInternetStackToAllNodes(nodes);
    }

    void FANETRoutingHelper::SetOLSR(NodeContainer nodes){
        OlsrHelper oslr;

        // Adjust OSLR parameters here
        
        this->list.Add(oslr, 100);
        InstallInternetStackToAllNodes(nodes);
    }

    void FANETRoutingHelper::SetDSDV(NodeContainer nodes){
        DsdvHelper dsdv;

        // Adjust dsdv parameters here
        
        this->list.Add(dsdv, 100);
        InstallInternetStackToAllNodes(nodes);
    }

    // void FANETRoutingHelper::SetDSR(NodeContainer nodes){
    //     DsrHelper dsr;

    //     // Adjust dsr parameters here
        
    //     this->list.Add(dsr, 100);
    //     InstallInternetStackToAllNodes(nodes);
    // }
}