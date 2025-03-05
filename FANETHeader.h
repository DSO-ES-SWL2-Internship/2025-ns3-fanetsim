#include "ns3/header.h"       // Base class for headers
#include "ns3/type-id.h"      // For defining TypeId
#include "ns3/buffer.h"       // For serialization and deserialization
#include "ns3/log.h"          // For NS_LOG_INFO and debugging


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

    };
}