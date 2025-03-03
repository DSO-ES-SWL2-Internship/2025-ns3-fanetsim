#ifndef FANET_APP_HELPER_H
#define FANET_APP_HELPER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/node.h"
#include "ns3/application.h"

namespace ns3
{
    template <typename T, typename... Args>
    void InstallApplication(Ptr<Node> node, double startTime, double endTime, Args&&... setupFunctions)
    {
        Ptr<T> app = CreateObject<T>();
        node->AddApplication(app);

        // Expand and call each setup function
        (setupFunctions(app), ...);

        app->SetStartTime(Seconds(startTime));
        app->SetStopTime(Seconds(endTime));
    }
}



#endif