#include "fanet-application.h"
#include "ns3/fanet-communication.h"
#include "ns3/simulator.h"
#include <cstring>

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
            SendPLRRequest(destAddress);
        }
    }

    void FANETApplication::SchedulePLR(double time, Ipv4Address destAddress, uint32_t pktsToSend, double interval)
    {
        Simulator::Schedule(Seconds(time), &FANETApplication::SendPLRPacket, this, destAddress, pktsToSend, interval);
    }

    void FANETApplication::HandlePLRPacket(FANETHeader* header, Ptr<Packet> packet, Address from)
    {
        uint32_t seqNum;
        packet->CopyData((uint8_t*) &seqNum, sizeof(uint32_t));

        // handling for correct sequence of plr packets
        if (seqNum >= m_expectedSeqPLR)
        {
            m_lostPacketsPLR += (seqNum - m_expectedSeqPLR);
            m_receivedPacketsPLR++;
        } 
        // handling for incorrect sequence of plr packets e.g. receive seq 2->3->4->1
        // if this occured, it means that the other node has scheduled a new PLR test
        else 
        {
            NS_LOG_INFO("New PLR test detected. Resetting counters.");
            m_lostPacketsPLR = seqNum == 0 ? 0 : seqNum;  //reset and account for the packets that could have been lost at the start of new test
            m_receivedPacketsPLR = 1;  // Start counting received packets
        }
            
        m_expectedSeqPLR = seqNum + 1;


        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
            << GetNode()->GetId() << " received PLR packet from " << header->GetNodeId() << " . Current PLR: " << GetPLR() << "%");
    }

    void FANETApplication::ScheduleRequestPLR(double time, Ipv4Address destAddress)
    {
        Simulator::Schedule(Seconds(time), &FANETApplication::SendPLRRequest, this, destAddress);
    }

    void FANETApplication::HandlePLRRequest(FANETHeader* header, Ptr<Packet> packet, Address from)
    {
        Ipv4Address senderIp = InetSocketAddress::ConvertFrom(from).GetIpv4();
        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() 
            << "s, Node " << GetNode()->GetId() 
            << " received PLR request from " << senderIp);
        
        SendPLRResponse(senderIp);
    }

    void FANETApplication::HandlePLRResponse(FANETHeader* header, Ptr<Packet> packet, Address from)
    {
        double plr;
        packet->CopyData(reinterpret_cast<uint8_t*>(&plr), sizeof(plr));

        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
            << GetNode()->GetId() << " received PLR from Node " << header->GetNodeId() << ". PLR: " << plr * 100.0 << "%");
    }

    void FANETApplication::SendPLRResponse(Ipv4Address destAddress)
    {
        FANETHeader header;
        header.SetType(RESPONSE);
        header.SetService(PLR);
        header.SetClusterId(9999);
        header.SetNodeId(GetNode()->GetId());

        double plr = GetPLR();
        Ptr<Packet> packet = Create<Packet>((uint8_t*) &plr, sizeof(double));
        packet->AddHeader(header);
        
        FANETCommunication::SendPacket(this, packet, destAddress);
        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() 
            << "s, Node " << GetNode()->GetId() 
            << " sent PLR response to " << destAddress);
    
        m_expectedSeqPLR = 0;
        m_lostPacketsPLR = 0;
    }

    void FANETApplication::SendPLRRequest(Ipv4Address destAddress)
    {
        FANETHeader header;
        header.SetType(REQUEST);
        header.SetService(PLR);
        header.SetClusterId(9999);
        header.SetNodeId(GetNode()->GetId());

        Ptr<Packet> packet = Create<Packet>();
        packet->AddHeader(header);

        FANETCommunication::SendPacket(this, packet, destAddress);
                NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() 
            << "s, Node " << GetNode()->GetId() 
            << " sent PLR request to " << destAddress);
    }
}