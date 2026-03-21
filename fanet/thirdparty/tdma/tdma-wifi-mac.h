#ifndef TDMA_WIFI_MAC_H
#define TDMA_WIFI_MAC_H

#include "ns3/wifi-mac.h"
#include "ns3/wifi-mac-header.h"

#include <queue>

namespace ns3
{

    struct TdmaBufferItem
    {
        Ptr<WifiMpdu> mpdu; // The packet to be transmitted
        WifiMacHeader hdr;  // The MAC header for the packet
        Mac48Address to;    // The destination address
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

        private:
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