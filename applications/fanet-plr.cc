#include "fanet-application.h"
#include "ns3/fanet-communication.h"
#include "ns3/simulator.h"
#include <cstring>

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("FANETPlr");

    void FANETApplication::SetupPLR(uint32_t nNodes)
    {
        m_sequenceNumberPLR.assign(nNodes, 0);
        m_expectedSeqPLR.assign(nNodes, 0);
        m_receivedPacketsPLR.assign(nNodes, 0);
        m_lostPacketsPLR.assign(nNodes, 0);
        m_packetsSentPLR.assign(nNodes, 0);
    }

    void FANETApplication::StartPLRTest(double startTime, uint32_t nodeId, Ipv4Address destAddress, uint32_t pktsToSend, double interval)
    {
        Simulator::Schedule(Seconds(startTime), &FANETApplication::SendPLRPacket, this, nodeId, destAddress, pktsToSend, interval);
    }

    void FANETApplication::HandlePLRPacket(FANETHeader* header, Ptr<Packet> packet, Address from)
    {
        uint32_t seqNum;
        packet->CopyData((uint8_t*) &seqNum, sizeof(uint32_t));

        uint32_t nodeId = header->GetNodeId();

        // handling for correct sequence of plr packets
        if (seqNum >= m_expectedSeqPLR[nodeId])
        {
            m_lostPacketsPLR[nodeId] += (seqNum - m_expectedSeqPLR[nodeId]);
            m_receivedPacketsPLR[nodeId]++;
        } 
        // handling for incorrect sequence of plr packets e.g. receive seq 2->3->4->1
        // if this occured, it means that the other node has scheduled a new PLR test
        else 
        {
            NS_LOG_INFO("New PLR test detected. Resetting counters.");
            m_lostPacketsPLR[nodeId] = seqNum == 0 ? 0 : seqNum;  //reset and account for the packets that could have been lost at the start of new test
            m_receivedPacketsPLR[nodeId] = 1;  // Start counting received packets
        }
            
        m_expectedSeqPLR[nodeId] = seqNum + 1;


        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
            << GetNode()->GetId() << " received PLR packet from " << header->GetNodeId() << " (Seq no. = " << seqNum << ") . Current PLR: " << GetPLR(nodeId) * 100 << "%");
    }

    void FANETApplication::HandlePLRRequest(FANETHeader* header, Ptr<Packet> packet, Address from)
    {
        Ipv4Address senderIp = InetSocketAddress::ConvertFrom(from).GetIpv4();
        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() 
            << "s, Node " << GetNode()->GetId() 
            << " received PLR request from Node " << header->GetNodeId());
        
        SendPLRResponse(header->GetNodeId(), senderIp);
    }

    void FANETApplication::HandlePLRResponse(FANETHeader* header, Ptr<Packet> packet, Address from)
    {
        double plr;
        packet->CopyData(reinterpret_cast<uint8_t*>(&plr), sizeof(plr));

        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
            << GetNode()->GetId() << " received PLR from Node " << header->GetNodeId() << ". PLR: " << plr * 100.0 << "%");
    }

    void FANETApplication::SendPLRRequest(uint32_t nodeId, Ipv4Address destAddress)
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
            << " sent PLR request to " << nodeId);
    }

    double FANETApplication::GetPLR(uint32_t nodeId) { return (double) m_lostPacketsPLR[nodeId] / (m_receivedPacketsPLR[nodeId] + m_lostPacketsPLR[nodeId]); }

    void FANETApplication::SendPLRPacket(uint32_t nodeId, Ipv4Address destAddress, uint32_t pktsToSend, double interval)
    {
        if (m_packetsSentPLR[nodeId] < pktsToSend)
        {
            FANETHeader header;
            header.SetType(DATA);
            header.SetService(PLR);
            header.SetClusterId(9999);
            header.SetNodeId(GetNode()->GetId());

            Ptr<Packet> packet = Create<Packet>((uint8_t*) &m_sequenceNumberPLR[nodeId], sizeof(uint32_t));
            packet->AddHeader(header);
            
            FANETCommunication::SendPacket(this, packet, destAddress);
            NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s, Node " 
                << GetNode()->GetId() << " sent PLR packet to destination (Seq No. = " 
                << m_sequenceNumberPLR[nodeId] << ") Node " << nodeId);

            m_sequenceNumberPLR[nodeId]++;
            m_packetsSentPLR[nodeId]++;

            Simulator::Schedule(Seconds(interval), &FANETApplication::SendPLRPacket, this, nodeId, destAddress, pktsToSend, interval);
        } 
        else 
        {
            m_packetsSentPLR[nodeId] = 0;
            NS_LOG_DEBUG("All PLR packets have been sent");
            SendPLRRequest(nodeId, destAddress);
        }
    }

    void FANETApplication::SendPLRResponse(uint32_t nodeId, Ipv4Address destAddress)
    {
        FANETHeader header;
        header.SetType(RESPONSE);
        header.SetService(PLR);
        header.SetClusterId(9999);
        header.SetNodeId(GetNode()->GetId());

        double plr = GetPLR(nodeId);
        Ptr<Packet> packet = Create<Packet>((uint8_t*) &plr, sizeof(double));
        packet->AddHeader(header);
        
        FANETCommunication::SendPacket(this, packet, destAddress);
        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() 
            << "s, Node " << GetNode()->GetId() 
            << " sent PLR response to Node " << nodeId);
    
        m_expectedSeqPLR[nodeId] = 0;
        m_lostPacketsPLR[nodeId] = 0;
    }
}