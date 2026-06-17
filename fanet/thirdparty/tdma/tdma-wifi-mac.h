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

    /// Structure to represent a mini-slot in the TDMA schedule
    struct MiniSlot 
    {
    bool isOccupied; // Indicates if the mini-slot is occupied
    std::string trafficType; // Type of traffic assigned to this mini-slot (e.g., "video", "audio", "data")
    };

    struct TrafficProfile 
    {
    std::string type;
    uint32_t priority; // Higher value means higher priority
    double bandwidthKb; // Bandwidth requirement in Kb/s for this traffic type
    };

    //Configuration structure for the TDMA MAC, which can be populated from JSON data
    struct ClusterMacConfig 
    {
        std::vector<TrafficProfile> trafficProfiles;
        uint32_t totalMiniSlots = 12;
        double kbPerMiniSlot = 0.1;
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

            void AllocateMiniSlots();

            std::string GetSlotTrafficType(uint32_t slotId) const;  

            // Function to set the cluster configuration, which includes traffic profiles and mini-slot allocation 
            //(to wire up the reference from the JSON data to the MAC layer))
            void SetClusterConfig(const ClusterMacConfig* sharedConfig);
            
        private:
            std::vector<MiniSlot> m_allocationTable;
        
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
            uint32_t m_totalMiniSlots;   
            double m_kbPerMiniSlot;
            const ClusterMacConfig* m_clusterConfig = nullptr; // Pointer to the shared cluster configuration

            //MAC-Level WFQ Queues
            std::queue<TdmaBufferItem> m_videoQueue;
            std::queue<TdmaBufferItem> m_statusQueue;
            std::queue<TdmaBufferItem> m_cmdQueue;

            //Leaky Bucket Limits (Max packets allowed to wait)
            uint32_t m_maxVideoQueueSize = 50;
            uint32_t m_maxStatusQueueSize = 50;
            uint32_t m_maxCmdQueueSize = 50;
    };
}
#endif