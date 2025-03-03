#ifndef FANET_APP_HELPER_H
#define FANET_APP_HELPER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/node.h"
#include "ns3/application.h"

namespace ns3
{
    /**
     * @brief Installs an application on an NS-3 node.
     * 
     * This template function creates an instance of the specified application type,
     * adds it to the given node, and sets its start and stop times. Additional setup
     * functions can be provided to configure the application.
     * 
     * @tparam T The type of the application to be installed (must inherit from ns3::Application).
     * @tparam Args Variadic template parameters for setup functions.
     * @param node The NS-3 node on which to install the application.
     * @param startTime The simulation time (in seconds) at which the application should start.
     * @param endTime The simulation time (in seconds) at which the application should stop.
     * @param setupFunctions One or more setup functions that take a Ptr<T> and configure the application.
     */
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