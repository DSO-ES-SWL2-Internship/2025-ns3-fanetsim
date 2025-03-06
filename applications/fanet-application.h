#ifndef FANET_APPLICATION_H
#define FANET_APPLICATION_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include <queue>

namespace ns3
{
    class FANETApplication : public Application  // Inherit from ns3::Application
    {
    public:
        FANETApplication();
        virtual ~FANETApplication();

        //virtual void HandleRead() = 0;         // Must be implemented by derived classes
        virtual void ProcessNextPacket() = 0;  // Must be implemented by derived classes

    protected:
        virtual void StartApplication() override = 0; // Ensure derived class implements
        virtual void StopApplication() override = 0;  // Ensure derived class implements

        /// @brief UDP socket for receiving and sending data
        Ptr<Socket> m_socket;
        /// @brief  Destination IP Address to send packets to
        Ipv4Address m_destAddr;
        /// @brief Port to listen to
        uint16_t m_port;
        /// @brief Queue to store received packets for processing
        std::queue<Ptr<Packet>> m_packetQueue;
        /// @brief  Queue to store sender addresses associated with received packets
        std::queue<Address> m_addressQueue;
    };
}

#endif
