#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "fanet_utils.h"
#include "ns3/FANETSimulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MANETNetwork");


int main (int argc, char* argv[]){
    bool verbose = true;

    if (verbose){
        LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
        //LogComponentEnable ("FANETTopologyHelper", LOG_LEVEL_INFO);
        //LogComponentEnable ("FANETMobilityHelper", LOG_LEVEL_INFO);
        LogComponentDisable("FANETDeviceHelper", LOG_LEVEL_DEBUG);
        LogComponentEnable ("FANETDeviceHelper", LOG_LEVEL_INFO);
        //LogComponentEnable ("FANETDeviceHelper", LOG_LEVEL_DEBUG);
        //LogComponentEnable("FANETRoutingHelper", LOG_LEVEL_INFO);

    }

    FANETSimulator sim;

    sim.RunSimulation("test.xml");



    return 0;
}