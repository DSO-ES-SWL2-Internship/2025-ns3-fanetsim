#ifndef TDMA_WIFI_MAC_H
#define TDMA_WIFI_MAC_H

#include "ns3/wifi-mac.h"
#include "ns3/wifi-mac-header.h"
#include <string>
#include <vector>
#include <queue>

namespace ns3
{

    struct TdmaBufferItem
    {
        Ptr<WifiMpdu> mpdu; // The packet to be transmitted
        WifiMacHeader hdr;  // The MAC header for the packet
        Mac48Address to;    // The destination address
    };

    struct MiniSlot {
    bool isOccupied;
    std::string trafficType;
    };

    struct TrafficProfile {
    std::string type;
    uint32_t priority;
    double bandwidthKb;
    };

    class TdmaWifiMac : public WifiMac
    {
        public:
            static TypeId GetTypeId();
            TdmaWifiMac();
            virtual ~TdmaWifiMac();

            void SetTdmaParameters(uint32_t numSlots, Time cycleDuration, uint32_t assignedSlot);
            void StartTdma();
            void Enqueue(Ptr<WifiMpdu> mpdu, Mac48Address to, Mac48Address from) override;
            // void Enqueue(Ptr<Packet> packet, Mac48Address to) override;
            bool CanForwardPacketsTo(Mac48Address to) const override;

            // A function to receive the JSON data from the simulator
            void SetTrafficProfiles(std::vector<TrafficProfile> profiles);
            void AllocateMiniSlots();

        private:
            std::vector<TrafficProfile> m_macTrafficProfiles;
            std::vector<MiniSlot> m_allocationTable;
    
            uint32_t m_totalMiniSlots = 12; 
            double m_kbPerMiniSlot = 0.1;   // 0.1K per slot
        
            void TdmaScheduleNextSlot();
            void TdmaTransmit();
            void UpdateSlotDuration();
            void Receive(Ptr<const WifiMpdu> mpdu, uint8_t linkId) override;
            void DoCompleteConfig() override;

            uint32_t m_numSlots;          // Total number of TDMA slots (equal to the number of nodes)
            Time m_cycleDuration;         // Duration of one TDMA cycle
            Time m_slotDuration;          // Duration of each slot (calculated as cycleDuration / numSlots)
            uint32_t m_assignedSlot;      // Slot assigned to this node
            uint32_t m_currentSlot;       // Current slot in the TDMA cycle
            EventId m_tdmaEvent;          // Event for scheduling the next slot
            bool m_isMySlot;              // Flag to indicate if it's the node's slot

            std::queue<TdmaBufferItem> m_tdmaBuffer;
    };
};

#endif