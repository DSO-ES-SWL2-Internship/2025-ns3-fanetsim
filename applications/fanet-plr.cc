#include "fanet-application.h"
#include "ns3/fanet-communication.h"
#include "ns3/simulator.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("FANETPlr");

    void FANETApplication::SetupPLRSender(Ipv4Address address, double interval)
    {
        m_destAddrPLR = address;
        m_intervalPLR = interval;
    }

    double FANETApplication::GetPLR() { return (double) m_lostPacketsPLR / (m_receivedPacketsPLR + m_lostPacketsPLR); }

    void FANETApplication::SendPLRPacket()
    {
        FANETHeader header;
        header.SetType(DATA);
        header.SetService(PLR);
        header.SetClusterId(9999);
        header.SetNodeId(GetNode()->GetId());

        Ptr<Packet> packet = Create<Packet>(sizeof(uint32_t));
        packet->CopyData((uint8_t*) &m_sequenceNumberPLR, sizeof(uint32_t));
        
        FANETCommunication::SendPacket(this, packet, m_destAddrPLR);
        m_sequenceNumberPLR++;

        Simulator::Schedule(Seconds(m_intervalPLR), &FANETApplication::SendPLRPacket, this);
    }

    void FANETApplication::HandlePLRPacket(FANETHeader* header, Ptr<Packet> packet)
    {
        uint32_t seqNum;
        packet->CopyData((uint8_t*) &seqNum, sizeof(uint32_t));

        if (seqNum > m_expectedSeqPLR)
            m_lostPacketsPLR += (seqNum - m_expectedSeqPLR);

        m_receivedPacketsPLR++;
        m_expectedSeqPLR = seqNum + 1;
    }

}