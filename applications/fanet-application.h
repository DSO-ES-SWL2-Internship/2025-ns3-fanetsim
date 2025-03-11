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

            /**
             * @brief Sets the destination IPv4 address.
             * 
             * This function updates the destination address used for communication.
             * 
             * @param destAddr The IPv4 address to be set as the destination.
             */
            void SetDestAddr(Ipv4Address destAddr);

            /**
             * @brief Sets the communication port.
             * 
             * This function assigns the specified port number for communication.
             * 
             * @param port The port number to be set.
             */
            void SetPort(uint16_t port);

            /**
             * @brief Retrieves the socket instance.
             * 
             * This function returns the socket used for communication.
             * 
             * @return A pointer to the socket.
             */
            Ptr<Socket> GetSocket();

            /**
             * @brief Retrieves the destination IPv4 address.
             * 
             * This function returns the currently set destination address.
             * 
             * @return The destination IPv4 address.
             */
            Ipv4Address GetDestAddr();

            /**
             * @brief Retrieves the communication port.
             * 
             * This function returns the currently assigned port number.
             * 
             * @return The port number.
             */
            uint16_t GetPort();

            /**
             * @brief Retrieves the packet queue.
             * 
             * This function returns a reference to the queue holding packets to be processed.
             * 
             * @return A reference to the packet queue.
             */
            std::queue<Ptr<Packet>>& GetPacketQueue();

            /**
             * @brief Retrieves the address queue.
             * 
             * This function returns a reference to the queue holding addresses associated with packets.
             * 
             * @return A reference to the address queue.
             */
            std::queue<Address>& GetAddressQueue();

            /**
             * @brief Processes the next packet in the queue.
             * 
             * This function retrieves and processes the next packet from the packet queue.
             * It removes the packet from the queue, extracts its header, and dispatches it 
             * to the appropriate handler based on the packet type (HELLO, DATA, REQUEST, RESPONSE).
             * If the queue is not empty after processing, it schedules the next packet for processing.
             */
            void ProcessNextPacket(); 

            virtual void EnableInfoLog();
            virtual void EnableDebugLog();

            void ScheduleRequestPLR(double time, Ipv4Address destAddress);

            void SchedulePLR(double time, Ipv4Address destAddress, uint32_t pktsToSend, double interval);

            

        protected:
            /**
             * @brief Starts the application.
             * 
             * This is a pure virtual function that must be implemented by derived classes. 
             * It initializes and starts the necessary components for the application.
             */    
            virtual void StartApplication() override = 0; 


            /**
             * @brief Stops the application.
             * 
             * This is a pure virtual function that must be implemented by derived classes. 
             * It handles cleanup and stops the application gracefully.
             */
            virtual void StopApplication() override = 0;  

            /**
             * @brief Registers packet handlers.
             * 
             * This is a pure virtual function that must be implemented by derived classes. 
             * It sets up the appropriate handlers for processing different types of packets.
             */
            virtual void RegisterHandlers();

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
            /// @brief unordered map to store hello service-handler key-value pairs
            std::unordered_map<int, std::function<void(FANETHeader*, Ptr<Packet>, Address)>> helloServiceHandlers;
            /// @brief unordered map to store data service-handler key-value pairs
            std::unordered_map<int, std::function<void(FANETHeader*, Ptr<Packet>, Address)>> dataServiceHandlers;
            /// @brief unordered map to store request service-handler key-value pairs
            std::unordered_map<int, std::function<void(FANETHeader*, Ptr<Packet>, Address)>> requestServiceHandlers;
            /// @brief unordered map to store response service-handler key-value pairs
            std::unordered_map<int, std::function<void(FANETHeader*, Ptr<Packet>, Address)>> responseServiceHandlers;     

            /**
             * @brief Processes a received Hello packet.
             * 
             * This function looks up the corresponding service handler for the given Hello packet
             * based on the service type in the FANETHeader. If a handler is found, it is invoked;
             * otherwise, a log message is printed indicating that no handler is registered.
             * 
             * @param header Pointer to the FANETHeader containing packet metadata.
             * @param packet Pointer to the received packet.
             */
            void ProcessHelloPacket(FANETHeader* header, Ptr<Packet> packet, Address from);

            /**
             * @brief Processes a received Data packet.
             * 
             * This function checks the data service handlers for a corresponding handler
             * based on the service type in the FANETHeader. If a handler is found, it is invoked;
             * otherwise, a log message is printed.
             * 
             * @param header Pointer to the FANETHeader containing packet metadata.
             * @param packet Pointer to the received packet.
             */
            void ProcessDataPacket(FANETHeader* header, Ptr<Packet> packet, Address from);


            /**
             * @brief Processes a received Request packet.
             * 
             * This function looks up the appropriate request service handler based on the 
             * service type in the FANETHeader. If a handler exists, it is executed; otherwise,
             * a log message is printed indicating that no handler is registered.
             * 
             * @param header Pointer to the FANETHeader containing packet metadata.
             * @param packet Pointer to the received packet.
             */
            void ProcessRequestPacket(FANETHeader* header, Ptr<Packet> packet, Address from);

            /**
             * @brief Processes a received Response packet.
             * 
             * This function checks if a corresponding response service handler exists
             * for the given service type in the FANETHeader. If a handler is registered,
             * it is invoked; otherwise, a log message is generated.
             * 
             * @param header Pointer to the FANETHeader containing packet metadata.
             * @param packet Pointer to the received packet.
             */
            void ProcessResponsePacket(FANETHeader* header, Ptr<Packet> packet, Address from);

            void HandlePLRPacket(FANETHeader* header, Ptr<Packet> packet, Address from);

            void SendPLRPacket(Ipv4Address destAddress, uint32_t pktsToSend, double interval);

            double GetPLR();

            void HandlePLRRequest(FANETHeader* header, Ptr<Packet> packet, Address from);

            void HandlePLRResponse(FANETHeader* header, Ptr<Packet> packet, Address from);

            void SendPLRResponse(Ipv4Address destAddress);

        private:
            uint32_t m_packetSizePLR;
            uint32_t m_sequenceNumberPLR;
            uint32_t m_expectedSeqPLR;
            uint32_t m_receivedPacketsPLR;
            uint32_t m_lostPacketsPLR;
            uint32_t m_packetsSentPLR;
            
    };
}

#endif
