#include "FANETAddressHelper.h"

namespace ns3
{
    FANETAddressHelper::FANETAddressHelper(){

    }

    FANETAddressHelper::FANETAddressHelper(Ipv4Address network, Ipv4Mask mask)
        : network(network), mask(mask)
    {

    }

    void FANETAddressHelper::SetUp(Ipv4Address network, Ipv4Mask mask){
        this->network = network;
        this->mask = mask;
    }

    FANETAddressHelper::~FANETAddressHelper(){

    }

    void FANETAddressHelper::IncrementNetwork(){
        uint32_t address = network.Get();

        // Extract the octets
        uint8_t octet1 = (address >> 24) & 0xFF;
        uint8_t octet2 = (address >> 16) & 0xFF;
        uint8_t octet3 = (address >> 8) & 0xFF;
        uint8_t octet4 = address & 0xFF;

        // Increment the third octet
        octet3++;   

        // Reconstruct the IP address with last octet set to 0
        network = Ipv4Address((octet1 << 24) | (octet2 << 16) | (octet3 << 8) | 0);   

        return;
    }

    void FANETAddressHelper::SetBases(std::vector<NetDeviceContainer> clustersDevices, std::vector<NetDeviceContainer> GDTtoCHLinksDevices){
        for (size_t i = 0; i < clustersDevices.size(); i++){
            ipv4.SetBase(network, mask);
            Ipv4InterfaceContainer clusterInterface = ipv4.Assign(clustersDevices[i]);
            clustersInterfaces.push_back(clusterInterface);
            IncrementNetwork();
        }

        for (size_t i = 0; i < GDTtoCHLinksDevices.size(); i++){
            ipv4.SetBase(network, mask);
            Ipv4InterfaceContainer linkInterface = ipv4.Assign(GDTtoCHLinksDevices[i]);
            GDTtoCHLinksInterfaces.push_back(linkInterface);  
            IncrementNetwork();
        }

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }
}