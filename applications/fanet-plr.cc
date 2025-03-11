#include "fanet-application.h"
#include "ns3/fanet-communication.h"
#include "ns3/simulator.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("FANETPlr");

    double FANETApplication::GetPLR() { return (double) m_lostPacketsPLR / (m_receivedPacketsPLR + m_lostPacketsPLR); }

    void FANETApplication::SendPLRPacket(Ipv4Address destAddress, uint32_t pktsToSend, double interval)
    {
        if (m_packetsSentPLR < pktsToSend)
        {
            FANETHeader header;
            header.SetType(DATA);
            header.SetService(PLR);
            header.SetClusterId(9999);
            header.SetNodeId(GetNode()->GetId());

            Ptr<Packet> packet = Create<Packet>((uint8_t*) &m_sequenceNumberPLR, sizeof(uint32_t));
            packet->AddHeader(header);
            
            FANETCommunication::SendPacket(this, packet, destAddress);
            NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
                << GetNode()->GetId() << " sent PLR packet to destination (Seq No. = " 
                << m_sequenceNumberPLR << "): " << destAddress);

            m_sequenceNumberPLR++;
            m_packetsSentPLR++;

            Simulator::Schedule(Seconds(interval), &FANETApplication::SendPLRPacket, this, destAddress, pktsToSend, interval);
        } 
        else 
        {
            m_packetsSentPLR = 0;
            NS_LOG_DEBUG("All PLR packets have been sent");
        }
    }

    void FANETApplication::SchedulePLR(double time, Ipv4Address destAddress, uint32_t pktsToSend, double interval)
    {
        Simulator::Schedule(Seconds(time), &FANETApplication::SendPLRPacket, this, destAddress, pktsToSend, interval);
    }

    void FANETApplication::HandlePLRPacket(FANETHeader* header, Ptr<Packet> packet)
    {
        uint32_t seqNum;
        packet->CopyData((uint8_t*) &seqNum, sizeof(uint32_t));

        if (seqNum > m_expectedSeqPLR)
            m_lostPacketsPLR += (seqNum - m_expectedSeqPLR);

        m_receivedPacketsPLR++;
        m_expectedSeqPLR = seqNum + 1;

        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
            << GetNode()->GetId() << " received PLR from " << header->GetNodeId() << " . Current PLR: " << GetPLR() << "%");
    }
}