#ifndef FANET_ADDRESS_HELPER_H
#define FANET_ADDRESS_HELPER_H

#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"


#include <cstdint>
#include <vector>

namespace ns3
{
    class FANETAddressHelper {
        private:
            Ipv4Address network;
            Ipv4Mask mask;
            Ipv4AddressHelper ipv4;

            void IncrementNetwork();                                           
            
        public:
            Ipv4InterfaceContainer GDTInterface;
            std::vector<Ipv4InterfaceContainer> clustersInterfaces;
            //std::vector<Ipv4InterfaceContainer> GDTtoCHLinksInterfaces;
            std::vector<std::vector<Ipv4InterfaceContainer>> clustersLinkInterfaces;
            std::vector<std::vector<std::pair<Ptr<Ipv4>, uint32_t>>> linksInterfaces;

            FANETAddressHelper();                                              
            FANETAddressHelper(Ipv4Address network, Ipv4Mask mask);            
            ~FANETAddressHelper();                                             

            void SetUp(Ipv4Address network, Ipv4Mask mask);                    
            //void SetBases(std::vector<NetDeviceContainer> clustersDevices, std::vector<NetDeviceContainer> GDTtoCHLinksDevices);   
            void SetBases(NetDeviceContainer GDTDevice, std::vector<NetDeviceContainer> clustersDevices, std::vector<std::vector<NetDeviceContainer>> clustersLinkDevices);
            static Ipv4Address GetBaseAddress(Ipv4Address ip);
        };
}

#endif