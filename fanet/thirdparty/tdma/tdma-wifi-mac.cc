#include "tdma-wifi-mac.h"

#include "ns3/qos-txop.h"

#include "ns3/eht-capabilities.h"
#include "ns3/he-capabilities.h"
#include "ns3/ht-capabilities.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/vht-capabilities.h"
#include "ns3/core-module.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("TdmaWifiMac");

    NS_OBJECT_ENSURE_REGISTERED(TdmaWifiMac);

    TypeId TdmaWifiMac::GetTypeId()
    {
        static TypeId tid = TypeId("ns3::TdmaWifiMac")
                                .SetParent<WifiMac>()
                                .SetGroupName("Wifi")
                                .AddConstructor<TdmaWifiMac>()
                                .AddAttribute("NumSlots", "Number of TDMA slots (equal to the number of nodes)",
                                            UintegerValue(4),
                                            MakeUintegerAccessor(&TdmaWifiMac::m_numSlots),
                                            MakeUintegerChecker<uint32_t>())
                                .AddAttribute("CycleDuration", "Duration of one TDMA cycle",
                                            TimeValue(MilliSeconds(400)), // Example: 400ms cycle
                                            MakeTimeAccessor(&TdmaWifiMac::m_cycleDuration),
                                            MakeTimeChecker());
        return tid;
    }

    // the default please dont use
    TdmaWifiMac::TdmaWifiMac()
        : m_numSlots(4),
        m_cycleDuration(MilliSeconds(400)),
        m_assignedSlot(0),
        m_currentSlot(0),
        m_isMySlot(false)
    {
        NS_LOG_FUNCTION(this);
        UpdateSlotDuration(); // Initialize slot duration
        SetTypeOfStation(ADHOC_STA);
    }

    TdmaWifiMac::~TdmaWifiMac()
    {
        NS_LOG_FUNCTION(this);
    }

    void TdmaWifiMac::SetTdmaParameters(uint32_t numSlots, Time cycleDuration, uint32_t assignedSlot)
    {
        m_numSlots = numSlots;
        m_cycleDuration = cycleDuration;
        m_assignedSlot = assignedSlot;
        UpdateSlotDuration(); // Recalculate slot duration
    }

    void TdmaWifiMac::StartTdma()
    {
        NS_LOG_FUNCTION(this);
        m_currentSlot = 0;
        TdmaScheduleNextSlot();
    }

void TdmaWifiMac::Enqueue(Ptr<WifiMpdu> mpdu, Mac48Address to, Mac48Address from)
    {
        auto packet = mpdu->GetPacket();
        NS_LOG_FUNCTION(this << packet << to);

        // when new packet is to be sent, check if the destination is a new location
        // FIXED: Do not register broadcast addresses as brand new unicast stations
        if (!to.IsBroadcast() && GetWifiRemoteStationManager()->IsBrandNew(to))
        {
            // In ad hoc mode, we assume that every destination supports all the rates we support.
            // Register the station with all the different capabilities
            // HT (High Throughput)
            // VHT (Very High Throughput)
            // HE (High Efficiency)
            // EHT (Extremely High Throughput)
            // ensure that the mac layer can support different station types
            if (GetHtSupported(to))
            {
                GetWifiRemoteStationManager()->AddAllSupportedMcs(to);
                GetWifiRemoteStationManager()->AddStationHtCapabilities(
                    to,
                    GetHtCapabilities(SINGLE_LINK_OP_ID));
            }
            if (GetVhtSupported(SINGLE_LINK_OP_ID))
            {
                GetWifiRemoteStationManager()->AddStationVhtCapabilities(
                    to,
                    GetVhtCapabilities(SINGLE_LINK_OP_ID));
            }
            if (GetHeSupported())
            {
                GetWifiRemoteStationManager()->AddStationHeCapabilities(
                    to,
                    GetHeCapabilities(SINGLE_LINK_OP_ID));
            }
            if (GetEhtSupported())
            {
                GetWifiRemoteStationManager()->AddStationEhtCapabilities(
                    to,
                    GetEhtCapabilities(SINGLE_LINK_OP_ID));
            }
            GetWifiRemoteStationManager()->AddAllSupportedModes(to);
            //GetWifiRemoteStationManager()->RecordDisassociated(to);
        }

        // Creating and configuring the Mac Header

        WifiMacHeader& hdr = mpdu->GetHeader();

        // If we are not a QoS STA then we definitely want to use AC_BE to
        // transmit the packet. A TID of zero will map to AC_BE (through \c
        // QosUtilsMapTidToAc()), so we use that as our default here.
        uint8_t tid = 0;

        // For now, a STA that supports QoS does not support non-QoS
        // associations, and vice versa. In future the STA model should fall
        // back to non-QoS if talking to a peer that is also non-QoS. At
        // that point there will need to be per-station QoS state maintained
        // by the association state machine, and consulted here.

        // Setting QoS in the header if its supported else just use WIFI_MAC_DATA
        if (GetQosSupported())
        {
            hdr.SetType(WIFI_MAC_QOSDATA);
            hdr.SetQosAckPolicy(WifiMacHeader::NORMAL_ACK);
            hdr.SetQosNoEosp();
            hdr.SetQosNoAmsdu();
            // Transmission of multiple frames in the same TXOP is not
            // supported for now
            hdr.SetQosTxopLimit(0);

            // Fill in the QoS control field in the MAC header
            tid = GetTid(packet, hdr);
            // Any value greater than 7 is invalid and likely indicates that
            // the packet had no QoS tag, so we revert to zero, which will
            // mean that AC_BE is used.
            if (tid > 7)
            {
                tid = 0;
            }
            hdr.SetQosTid(tid);
        }
        else
        {
            hdr.SetType(WIFI_MAC_DATA);
        }

        if (GetHtSupported(to))
        {
            hdr.SetNoOrder(); // explicitly set to 0 for the time being since HT control field is not
                            // yet implemented (set it to 1 when implemented)
        }

        // Setting Address
        hdr.SetAddr1(to);
        hdr.SetAddr2(GetAddress());
        hdr.SetAddr3(GetBssid(0));
        hdr.SetDsNotFrom();
        hdr.SetDsNotTo();

        // FIXED: The VIP PASS for Broadcast packets (ARP, AODV RREQ, etc.)
        // Let them bypass the TDMA buffer so routing can establish instantly!
        if (to.IsBroadcast()) {
            if (GetQosSupported()) {
                GetQosTxop(tid)->Queue(mpdu);
            } else {
                GetTxop()->Queue(mpdu);
            }
            return; // Exit the function immediately, bypassing the TDMA queue
        }

        // Deprecated, since MPDU is supposed to contain all information
        // Since later version of NS3
        // TODO: Refactor for cleanliness
        TdmaBufferItem item;
        item.mpdu = mpdu;
        // item.hdr = hdr;
        // item.to = to;

        m_tdmaBuffer.push(item);

        if (m_isMySlot)
        {
            TdmaTransmit();
        }
    }

    // Set to always return true meaning that this MAC layer allows packet forwarding to any MAC address
    bool
    TdmaWifiMac::CanForwardPacketsTo(Mac48Address to) const
    {
        return true;
    }

    void TdmaWifiMac::TdmaScheduleNextSlot()
    {
        NS_LOG_FUNCTION(this);

        // Check if it's the node's slot
        m_isMySlot = (m_currentSlot == m_assignedSlot);

        // Schedule the next slot
        m_tdmaEvent = Simulator::Schedule(m_slotDuration, &TdmaWifiMac::TdmaScheduleNextSlot, this);

        // If it's the node's slot, start transmitting
        if (m_isMySlot)
        {
            TdmaTransmit();
        }

        // Increment the slot counter
        m_currentSlot = (m_currentSlot + 1) % m_numSlots;
    }

    void TdmaWifiMac::TdmaTransmit()
    {
        NS_LOG_FUNCTION(this);

        Time slotStartTime = Simulator::Now();

        // Transmit all packets in the buffer during the node's slot
        while (!m_tdmaBuffer.empty())
        {

            if (Simulator::Now() - slotStartTime >= m_slotDuration)
            {
                NS_LOG_WARN("Slot overrun detected! Stopping transmission.");
                break; // Stop if the slot time is exceeded
            }

            // Get the next packet and header from the buffer
            TdmaBufferItem item = m_tdmaBuffer.front();
            Ptr<WifiMpdu> mpdu = item.mpdu;
            // WifiMacHeader hdr = item.hdr;
            // Mac48Address to = item.to;

            // Queue the packet and header in the appropriate Txop or QosTxop
            if (GetQosSupported())
            {
                uint8_t tid = GetTid(mpdu->GetPacket(), mpdu->GetHeader());
                GetQosTxop(tid)->Queue(mpdu);
            }
            else
            {
                GetTxop()->Queue(mpdu);
            }

            // Remove the item from the buffer
            m_tdmaBuffer.pop();
        }
    }

    void TdmaWifiMac::UpdateSlotDuration()
    {
        NS_LOG_FUNCTION(this);
        m_slotDuration = m_cycleDuration / m_numSlots;
        NS_LOG_DEBUG("Slot duration updated to " << m_slotDuration.As(Time::MS));
    }

     //Inside tdma-wifi-mac.cc
    void TdmaWifiMac::SetTrafficProfiles(std::vector<TrafficProfile> profiles) {
        this->m_macTrafficProfiles = profiles;
        AllocateMiniSlots(); // Re-allocate mini-slots based on the new traffic profiles
    }

    void TdmaWifiMac::AllocateMiniSlots() {
        //Reset the table to 12 empty slots
        m_allocationTable.clear();
        m_allocationTable.resize(m_totalMiniSlots, {false, ""});
        
        uint32_t slotsAvailable = m_totalMiniSlots;

        //Loop through the JSON profiles (already sorted highest priority first)
        for (const auto& profile : m_macTrafficProfiles) {
            
            // Calculate how many mini-slots this traffic needs (e.g., 0.6K / 0.1 = 6 slots)
            uint32_t slotsNeeded = std::ceil(profile.bandwidthKb / m_kbPerMiniSlot);
            
            // If we don't have enough slots left, it gets whatever is remaining (Starvation)
            uint32_t slotsToAllocate = std::min(slotsNeeded, slotsAvailable);
            
            // Fill the slots in the table
            for (uint32_t i = 0; i < slotsToAllocate; i++) {
                // Find the next available empty slot
                for (auto& slot : m_allocationTable) {
                    if (!slot.isOccupied) {
                        slot.isOccupied = true;
                        slot.trafficType = profile.type;
                        slotsAvailable--;
                        break;
                    }
                }
            }

            // If the table is full, stop allocating! Lower priorities get dropped.
            if (slotsAvailable == 0) {
                NS_LOG_DEBUG("TDMA Slot Capacity Reached. Lower priorities starved.");
                break; 
            }
        }
            std::cout << "\n[TDMA VERIFICATION] Node MAC: " << GetAddress() << " allocated 12 mini-slots:" << std::endl;
            for (uint32_t i = 0; i < m_totalMiniSlots; i++) {
                if (m_allocationTable[i].isOccupied) {
                    std::cout << "  Slot " << i << ": " << m_allocationTable[i].trafficType << std::endl;
                } else {
                    std::cout << "  Slot " << i << ": [ IDLE ]" << std::endl;
                }
            }
            std::cout << "------------------------------------------------" << std::endl;
    }

    // Receive MAC protocol data unit (MPDU) and extract the source and destination address
    void
    TdmaWifiMac::Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId)
    {
        NS_LOG_FUNCTION(this << *mpdu << +linkId);
        const WifiMacHeader* hdr = &mpdu->GetHeader();
        NS_ASSERT(!hdr->IsCtl());
        Mac48Address from = hdr->GetAddr2();
        Mac48Address to = hdr->GetAddr1();

        // Check if the sender is a new station, and registers its capabilities if so
        if (GetWifiRemoteStationManager()->IsBrandNew(from))
        {
            // In ad hoc mode, we assume that every destination supports all the rates we support.
            if (GetHtSupported(to))
            {
                GetWifiRemoteStationManager()->AddAllSupportedMcs(from);
                GetWifiRemoteStationManager()->AddStationHtCapabilities(
                    from,
                    GetHtCapabilities(SINGLE_LINK_OP_ID));
            }
            if (GetVhtSupported(SINGLE_LINK_OP_ID))
            {
                GetWifiRemoteStationManager()->AddStationVhtCapabilities(
                    from,
                    GetVhtCapabilities(SINGLE_LINK_OP_ID));
            }
            if (GetHeSupported())
            {
                GetWifiRemoteStationManager()->AddStationHeCapabilities(
                    from,
                    GetHeCapabilities(SINGLE_LINK_OP_ID));
            }
            if (GetEhtSupported())
            {
                GetWifiRemoteStationManager()->AddStationEhtCapabilities(
                    from,
                    GetEhtCapabilities(SINGLE_LINK_OP_ID));
            }
            GetWifiRemoteStationManager()->AddAllSupportedModes(from);
            //GetWifiRemoteStationManager()->RecordDisassociated(from);
        }


        // If the received packet is QoS A-MSDU, it is deaggregated.
        // Otherwise, it is forwarded to higher layers.
        // If the packet is not a data packet, it is processed by the base class (WifiMac).
        if (hdr->IsData())
        {
            if (hdr->IsQosData() && hdr->IsQosAmsdu())
            {
                NS_LOG_DEBUG("Received A-MSDU from" << from);
                DeaggregateAmsduAndForward(mpdu);
            }
            else
            {
                ForwardUp(mpdu->GetPacket()->Copy(), from, to);
            }
            return;
        }

        // Invoke the receive handler of our parent class to deal with any
        // other frames. Specifically, this will handle Block Ack-related
        // Management Action frames.

        // if not a data packet, it will be processed by the base class
        WifiMac::Receive(mpdu, linkId);
    }

    //
    void TdmaWifiMac::DoCompleteConfig()
    {
        //
    }
}


