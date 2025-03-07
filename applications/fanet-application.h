#ifndef FANET_APPLICATION_H
#define FANET_APPLICATION_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include "ns3/FANETHeader.h"
#include <queue>
#include <unordered_map>
#include <functional>

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

        void ProcessNextPacket(); 

    protected:
        virtual void StartApplication() override = 0; // Ensure derived class implements
        virtual void StopApplication() override = 0;  // Ensure derived class implements
        virtual void RegisterHandlers() = 0;

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

        std::unordered_map<int, std::function<void(FANETHeader*, Ptr<Packet>)>> helloServiceHandlers;
        std::unordered_map<int, std::function<void(FANETHeader*, Ptr<Packet>)>> dataServiceHandlers;
        std::unordered_map<int, std::function<void(FANETHeader*, Ptr<Packet>)>> requestServiceHandlers;
        std::unordered_map<int, std::function<void(FANETHeader*, Ptr<Packet>)>> responseServiceHandlers;

        void RegisterHelloHandler(int serviceType, std::function<void(FANETHeader*, Ptr<Packet>)> handler);
        void RegisterDataHandler(int serviceType, std::function<void(FANETHeader*, Ptr<Packet>)> handler);
        void RegisterRequestHandler(int serviceType, std::function<void(FANETHeader*, Ptr<Packet>)> handler);
        void RegisterResponseHandler(int serviceType, std::function<void(FANETHeader*, Ptr<Packet>)> handler);

        //virtual void HandleRead() = 0;          

        void ProcessHelloPacket(FANETHeader* header, Ptr<Packet> packet);

        void ProcessDataPacket(FANETHeader* header, Ptr<Packet> packet);

        void ProcessRequestPacket(FANETHeader* header, Ptr<Packet> packet);

        void ProcessResponsePacket(FANETHeader* header, Ptr<Packet> packet);
    };
}

#endif
