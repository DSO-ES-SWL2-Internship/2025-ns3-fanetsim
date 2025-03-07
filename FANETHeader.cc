#include "FANETHeader.h"

namespace ns3 
{
    FANETHeader::FANETHeader()
        : m_type(HELLO), m_nodeId(0), m_clusterId(0), m_service(GENERAL)
    {

    }

    void FANETHeader::SetType(PacketType type) {m_type = type;}
    PacketType FANETHeader::GetType() {return m_type;}

    void FANETHeader::SetNodeId(uint32_t nodeId) {m_nodeId = nodeId;}
    uint32_t FANETHeader::GetNodeId() {return m_nodeId;}

    void FANETHeader::SetClusterId(uint32_t clusterId) {m_clusterId = clusterId;}
    uint32_t FANETHeader::GetClusterId() {return m_clusterId;}

    void FANETHeader::SetService(ServiceType service) {m_service = service;}
    ServiceType FANETHeader::GetService() {return m_service;}

    TypeId FANETHeader::GetTypeId() 
    {
        static TypeId tid = TypeId("ns3::FANETHeader")
            .SetParent<Header>()
            .AddConstructor<FANETHeader>();

        return tid;
    }

    TypeId FANETHeader::GetInstanceTypeId() const { return GetTypeId(); }

    uint32_t FANETHeader::GetSerializedSize() const { return sizeof(m_type) + sizeof(m_clusterId) + sizeof(m_nodeId) + sizeof(m_service); }

    void FANETHeader::Serialize(Buffer::Iterator start) const
    {
        start.WriteU8(static_cast<uint8_t>(m_type));
        start.WriteU32(static_cast<uint32_t>(m_nodeId));
        start.WriteU32(static_cast<uint32_t> (m_clusterId));
        start.WriteU8(static_cast<uint8_t>(m_service));
    }

    uint32_t FANETHeader::Deserialize(Buffer::Iterator start)
    {
        m_type = static_cast<PacketType> (start.ReadU8());
        m_nodeId = start.ReadU32();
        m_clusterId = start.ReadU32();
        m_service = static_cast<ServiceType> (start.ReadU8());

        return GetSerializedSize();
    }

    void FANETHeader::Print(std::ostream &os) const
    {
        os << "Type: " << m_type << ", Node ID: " << m_nodeId << ", Cluster ID: " 
            << m_clusterId << ", Service: " << m_service;
    }
}