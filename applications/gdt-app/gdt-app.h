#ifndef GDT_APP_H
#define GDT_APP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include <queue>

#include "ns3/fanet-application.h"

namespace ns3
{
    /**
     * @class GDTApp
     * @brief An application that manages the operations in a GDT
     */
    class GDTApp : public FANETApplication
    {
        public:
            /// @brief Constructor of GDTApp
            GDTApp();
            /// @brief Destructor of GDTApp
            ~GDTApp();

            /**
             * @brief Configure the application with the port number to listen to
             * @param the port number used for communication
             */
            void SetPort(uint16_t port);

            /// @brief Enable NS_LOG_INFO for this component
            void EnableInfoLog();
            
            /// @brief Enable NS_LOG_DEBUG for this component
            void EnableDebugLog();


        private:

            /// @brief Starts the application
            virtual void StartApplication() override;
            /// @brief Stop the application and close the socket
            virtual void StopApplication() override;

            /**
             * @brief Initialise the application
             * 
             * Creates the socket and bind it to m_port and set up the call back function
             * to handle incoming packets
             */
            virtual void DoInitialize() override;

            /**
             * @brief Callback function to handle incoming packets
             * 
             * This functions triggers when packet is received, storing it in the queue
             * for further processing
             * 
             * @param socket Socket that received the packet
             */
            void HandleRead(Ptr<Socket> socket);

            /**
             * @brief Process the next packet in the queue
             * 
             * Extracts the next packet from te queue and performs the necessary 
             * operations on it
             */
            void ProcessNextPacket();

    };
}


#endif