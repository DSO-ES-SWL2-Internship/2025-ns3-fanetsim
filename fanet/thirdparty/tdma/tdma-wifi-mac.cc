#include "tdma-wifi-mac.h"

#include "ns3/qos-txop.h"

#include "ns3/eht-capabilities.h"
#include "ns3/he-capabilities.h"
#include "ns3/ht-capabilities.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/vht-capabilities.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/mac48-address.h"
#include "ns3/packet.h"

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
                                            MakeTimeChecker())
                                .AddAttribute("TotalMiniSlots", "Total mini-slots in the frame format",
                                          UintegerValue(12),
                                          MakeUintegerAccessor(&TdmaWifiMac::m_totalMiniSlots),
                                          MakeUintegerChecker<uint32_t>())
                                .AddAttribute("KbPerMiniSlot", "Bandwidth weight per mini-slot unit",
                                          DoubleValue(0.1),
                                          MakeDoubleAccessor(&TdmaWifiMac::m_kbPerMiniSlot),
                                          MakeDoubleChecker<double>());
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

    // This method calculates the duration of each slot based on the total cycle duration and the number of slots.
    void TdmaWifiMac::SetTdmaParameters(uint32_t numSlots, Time cycleDuration, uint32_t assignedSlot)
    {
        m_numSlots = numSlots;
        m_cycleDuration = cycleDuration;
        m_assignedSlot = assignedSlot;
        UpdateSlotDuration(); // Recalculate slot duration
    }

    //
    void TdmaWifiMac::SetClusterConfig(const ClusterMacConfig* sharedConfig)
    {
        m_clusterConfig = sharedConfig;
    }

    // This method updates the slot duration whenever the number of slots or cycle duration changes.
    void TdmaWifiMac::StartTdma()
    {
        NS_LOG_FUNCTION(this);
        m_currentSlot = 0;
        TdmaScheduleNextSlot();
    }

    // This method calculates the duration of each slot based on the total cycle duration and the number of slots.
    void TdmaWifiMac::Enqueue(Ptr<WifiMpdu> mpdu, Mac48Address to, Mac48Address from)
    {
        auto packet = mpdu->GetPacket();
        NS_LOG_FUNCTION(this << packet << to);

        // Drop ALL packets (including AODV routing broadcasts) so the IP layer is forced to reroute.
        if (m_clusterConfig == nullptr) {
            return; 
        }

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
            // Some special TID values map to specific ACs (e.g. Video, Command, Status)
            if (tid > 7)
            {
                if (tid == 0xA0) tid = 5;      // Video
                else if (tid == 0xC0) tid = 6; // Cmd
                else if (tid == 0x80) tid = 4; // Status
                else tid = 0;                  // Best Effort Fallback
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
        // Read the TID mapped from the IP ToS byte
        // Use the safely mapped 'tid' from above to route to the correct Leaky Bucket
        if (tid == 3) { 
            if (m_videoQueue.size() >= m_maxVideoQueueSize) {
                NS_LOG_WARN("Video Leaky Bucket FULL! Dropping delayed frame.");
                return; 
            }
            m_videoQueue.push(item);
        } 
        else if (tid == 6) { 
            if (m_cmdQueue.size() >= m_maxCmdQueueSize) {
                NS_LOG_WARN("Command Leaky Bucket FULL! Dropping packet.");
                return;
            }
            m_cmdQueue.push(item);
        } 
        else { 
            if (m_statusQueue.size() >= m_maxStatusQueueSize) {
                NS_LOG_WARN("Status Leaky Bucket FULL! Dropping packet.");
                return;
            }
            m_statusQueue.push(item);
        }
        if (m_isMySlot) {
            TdmaTransmit();
        }
    }

    // Set to always return true meaning that this MAC layer allows packet forwarding to any MAC address
    bool
    TdmaWifiMac::CanForwardPacketsTo(Mac48Address to) const
    {
        return true;
    }

    // This method schedules the next slot in the TDMA cycle and checks if it's the node's assigned slot to transmit.
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

    // This method transmits all packets in the buffer during the node's assigned slot.
    // It checks for slot overruns to ensure that the node does not exceed its allocated time.
    void TdmaWifiMac::TdmaTransmit()
    {
        NS_LOG_FUNCTION(this);

        Time slotStartTime = Simulator::Now();
        
        //Evaluate how many packets of each traffic type we can send based on the allocation table for this slot
        uint32_t videoQuota = 0;
        uint32_t statusQuota = 0;
        uint32_t cmdQuota = 0;

        //Scans m_allocationTable and tallies up the weights for the current cycle
        for(const auto& slot : m_allocationTable) {
            if (!slot.isOccupied) {
                continue; // Skip empty slots
            }
            // Use find() so it matches "Video_HIGH_RES", "Video_LOW_RES", etc.
            if (slot.trafficType.find("Video") != std::string::npos) {
                videoQuota++;
            }
            else if (slot.trafficType.find("Status") != std::string::npos) {
                statusQuota++;
            }
            else if (slot.trafficType.find("Cmd") != std::string::npos) {
                cmdQuota++;
            }
        }

        //Lambda function to decrease a specific queue safely
        auto drainQueue = [&](std::queue<TdmaBufferItem>& queue, uint32_t& quota, uint8_t destTid) {
            while (!queue.empty() && quota > 0)
            {
                if (Simulator::Now() - slotStartTime >= m_slotDuration)
                {
                    NS_LOG_WARN("Slot overrun detected... Stopping transmission.");
                    return; 
                }
                TdmaBufferItem item = queue.front();
                Ptr mpdu = item.mpdu;

                if (GetQosSupported())
                {
                    // Directly use destTid instead of calculating it!
                    GetQosTxop(destTid)->Queue(mpdu);
                }
                else
                {
                    GetTxop()->Queue(mpdu);
                }
                queue.pop();
                quota--; 
            }
        };

        if (!m_videoQueue.empty() || !m_statusQueue.empty() || !m_cmdQueue.empty()) {
            std::cout << "[WFQ ENFORCER] Node Slot Active | Sending Max: " 
                      << videoQuota << " Video, " 
                      << statusQuota << " Status, " 
                      << cmdQuota << " Cmd." << std::endl;
        }    

        //Pass the explicit 802.11e TIDs: 6 (Cmd), 5 (Status), 3 (Video)
        drainQueue(m_cmdQueue, cmdQuota, 6); //Maps to AC_V0
        drainQueue(m_statusQueue, statusQuota, 5); //Maps to AC_VI
        drainQueue(m_videoQueue, videoQuota, 3); //Maps to AC_BE
    }

    // This method updates the slot duration whenever the number of slots or cycle duration changes.
    void TdmaWifiMac::UpdateSlotDuration()
    {
        NS_LOG_FUNCTION(this);
        m_slotDuration = m_cycleDuration / m_numSlots;
        NS_LOG_DEBUG("Slot duration updated to " << m_slotDuration.As(Time::MS));
    }

    // This method is called when a packet is received.
    // It extracts the source and destination addresses and processes the packet accordingly.
    void TdmaWifiMac::AllocateMiniSlots() {
        //check if the cluster configuration reference has been set before trying to allocate mini-slots
        if (!m_clusterConfig) {
            NS_LOG_WARN("No ClusterMacConfig reference attached yet!");
            return;
        }
        
        //Reset the table to 12 empty slots
        m_allocationTable.clear();
        m_allocationTable.resize(m_clusterConfig->totalMiniSlots, {false, ""});
        const std::vector<TrafficProfile> &profiles = m_clusterConfig->trafficProfiles;
        uint32_t slotsAvailable = m_clusterConfig->totalMiniSlots;


        //Loop through the JSON profiles (already sorted highest priority first)
        for (const auto& profile : profiles) 
        {
            //Calculate how many mini-slots this traffic needs (e.g., 0.6K / 0.1 = 6 slots)
            uint32_t slotsNeeded = std::ceil(profile.bandwidthKb / m_kbPerMiniSlot);
            
            //If we don't have enough slots left, it gets whatever is remaining (e.g., if only 4 slots left but needs 6, it gets 4 and is marked as partially allocated)
            uint32_t slotsToAllocate = std::min(slotsNeeded, slotsAvailable);
            
            //Fill the slots in the table accordingly with the traffic type and mark them as occupied
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

            // If the table is full, stop allocating. Lower priorities get dropped.
            if (slotsAvailable == 0) {
                break; 
            }
        }
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

    // This method returns the traffic type allocated to a specific slot ID. 
    //If the slot is not occupied, it returns "IDLE".
    std::string TdmaWifiMac::GetSlotTrafficType(uint32_t slotId) const
    {
        // Safety check to avoid out-of-bounds crashes
        if (slotId >= m_allocationTable.size()) 
        {
            return "IDLE";
        }
        
        // Return the traffic type if occupied, otherwise return "IDLE"
        return m_allocationTable[slotId].isOccupied ? m_allocationTable[slotId].trafficType : "IDLE";
    }
}


