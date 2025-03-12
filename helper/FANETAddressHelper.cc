#include "FANETAddressHelper.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("FANETAddressHelper");
    
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

    // void FANETAddressHelper::SetBases(std::vector<NetDeviceContainer> clustersDevices, std::vector<NetDeviceContainer> GDTtoCHLinksDevices){
    //     for (size_t i = 0; i < clustersDevices.size(); i++){
    //         ipv4.SetBase(network, mask);
    //         Ipv4InterfaceContainer clusterInterface = ipv4.Assign(clustersDevices[i]);
    //         clustersInterfaces.push_back(clusterInterface);
    //         IncrementNetwork();
    //     }

    //     for (size_t i = 0; i < GDTtoCHLinksDevices.size(); i++){
    //         ipv4.SetBase(network, mask);
    //         Ipv4InterfaceContainer linkInterface = ipv4.Assign(GDTtoCHLinksDevices[i]);
    //         GDTtoCHLinksInterfaces.push_back(linkInterface);  
    //         IncrementNetwork();
    //     }

    //     Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    // }

    void FANETAddressHelper::SetBases(NetDeviceContainer GDTDevice, std::vector<NetDeviceContainer> clustersDevices, std::vector<std::vector<NetDeviceContainer>> clustersLinkDevices)
    {


        for (size_t i = 0; i < clustersDevices.size(); i++){
            // Setup the interfaces for intra-cluster communication
            ipv4.SetBase(network, mask);
            Ipv4InterfaceContainer clusterInterface = ipv4.Assign(clustersDevices[i]);
            clustersInterfaces.push_back(clusterInterface);
            StoreClusterBaseIP(network);
            IncrementNetwork();
            
            // For each nodes in the cluster create the interface between the GDT and cluster node devices and disable them
            std::vector<Ipv4InterfaceContainer> clusterLinkInterfaces;
            for (size_t j = 0; j < clustersLinkDevices[i].size(); j++)
            {
                //ipv4.SetBase(linkNetwork, Ipv4Mask("255.255.255.252"));
                ipv4.SetBase(network, mask);
                IncrementNetwork();
                Ipv4InterfaceContainer linkInterface = ipv4.Assign(clustersLinkDevices[i][j]);
                
                Ptr<Ipv4> gdtIpv4Ptr = linkInterface.Get(0).first;
                Ptr<Ipv4> clusterNodeIpv4Ptr = linkInterface.Get(1).first;
                uint32_t gdtInterfaceIndex = linkInterface.Get(0).second;
                uint32_t clusterNodeInterfaceIndex = linkInterface.Get(1).second;

                gdtIpv4Ptr->SetDown(gdtInterfaceIndex);
                clusterNodeIpv4Ptr->SetDown(clusterNodeInterfaceIndex);

                clusterLinkInterfaces.push_back(linkInterface);

                //address += 4;
                //linkNetwork = Ipv4Address(address);
            }

            clustersLinkInterfaces.push_back(clusterLinkInterfaces);
            //IncrementNetwork();
        }

        // Setup the common interface on GDT that all cluster nodes can use to communicate with the GDT
        ipv4.SetBase(network, mask);
        IncrementNetwork();
        this->GDTInterface = ipv4.Assign(GDTDevice);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

    void FANETAddressHelper::StoreClusterBaseIP(Ipv4Address addr)
    {
        clustersBaseIP.push_back(addr);
    }

    Ipv4Address FANETAddressHelper::GetClusterBaseIP(uint32_t clusterIndex)
    {
        return clustersBaseIP[clusterIndex];
    }

    Ipv4Address FANETAddressHelper::GetBaseAddress(Ipv4Address ip)
    {
        uint32_t ipInt = ip.Get(); // Convert IP to integer

        // Mask out the last octet (set it to 0)
        uint32_t baseIPInt = ipInt & 0xFFFFFF00; 

        return Ipv4Address(baseIPInt); // Convert back to Ipv4Address
    }

    Ipv4Address FANETAddressHelper::GetBroadcastIP(Ipv4Address addr)
    {

        { return Ipv4Address( addr.Get() | 0x000000FF); }
    }
}