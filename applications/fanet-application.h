#ifndef FANET_APPLICATION_H
#define FANET_APPLICATION_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include "ns3/FANETHeader.h"
#include <queue>

namespace ns3
{
    class FANETApplication : public Application  // Inherit from ns3::Application
    {
    public:
        FANETApplication();
        virtual ~FANETApplication();

        void SetDestAddr(Ipv4Address destAddr);
        void SetPort(uint16_t port);
        Ptr<Socket> GetSocket();
        Ipv4Address GetDestAddr();
        uint16_t GetPort();
        std::queue<Ptr<Packet>>& GetPacketQueue();
        std::queue<Address>& GetAddressQueue();

        //virtual void HandleRead() = 0;         
        virtual void ProcessNextPacket() = 0;  

        virtual void ProcessHelloPacket(FANETHeader* header, Ptr<Packet> packet) = 0;

        virtual void ProcessDataPacket(FANETHeader* header, Ptr<Packet> packet) = 0;

        virtual void ProcessRequestPacket(FANETHeader* header, Ptr<Packet> packet) = 0;

        virtual void ProcessResponsePacket(FANETHeader* header, Ptr<Packet> packet) = 0;


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
