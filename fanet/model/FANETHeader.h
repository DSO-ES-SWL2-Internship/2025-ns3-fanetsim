#ifndef FANET_HEADER_H
#define FANET_HEADER_H

#include "ns3/header.h"       // Base class for headers
#include "ns3/type-id.h"      // For defining TypeId
#include "ns3/buffer.h"       // For serialization and deserialization
#include "ns3/log.h"          // For NS_LOG_INFO and debugging
#include "ns3/ipv4-address.h"

namespace ns3
{
    enum PacketType {
        HELLO,
        DATA,
        REQUEST,
        RESPONSE
    };

    enum ServiceType {
        GENERAL,
        PLR,
        CH_PROMO,
        OTHER_SERVICE
    };


    class FANETHeader : public Header
    {
        public:
            FANETHeader();

            void SetType(PacketType type);
            PacketType GetType();

            void SetNodeId(uint32_t nodeId);
            uint32_t GetNodeId();

            void SetClusterId(uint32_t clusterId);
            uint32_t GetClusterId();

            void SetService(ServiceType service);
            ServiceType GetService();

            void SetIsBroadcast(bool isBroadcast);
            bool GetIsBroadcast();

            void SetIsBroadcastForwarding(bool isBroadcastForwarding);
            bool GetIsBroadcastForwarding();

            void SetBroadcastFrom(Ipv4Address from);
            Ipv4Address GetBroadCastFrom();

            static TypeId GetTypeId();
            virtual TypeId GetInstanceTypeId() const override;
            virtual uint32_t GetSerializedSize() const override;
            virtual void Serialize(Buffer::Iterator start) const override;
            virtual uint32_t Deserialize(Buffer::Iterator start) override;
            virtual void Print(std::ostream &os) const override;
        
        private:
            PacketType m_type;
            uint32_t m_nodeId;
            uint32_t m_clusterId;
            ServiceType m_service;
            bool m_isBroadcast;
            bool m_isBroadcastForwarding;
            Ipv4Address m_broadcastFrom;

    };
}

#endif