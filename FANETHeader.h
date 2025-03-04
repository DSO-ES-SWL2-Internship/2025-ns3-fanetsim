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

    class FANETHeader : public Header
    {
        public:
            FANETHeader();
            void SetType(PacketType type);
            void SetNodeId(uint32_t nodeId);
            static TypeId GetTypeId();
            virtual TypeId GetInstanceTypeId() const override;
            virtual uint32_t GetSerializedSize() const override;

    };
}