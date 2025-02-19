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

            void IncrementNetwork();                                            //done
            
        public:
            std::vector<Ipv4InterfaceContainer> clustersInterfaces;
            std::vector<Ipv4InterfaceContainer> GDTtoCHLinksInterfaces;

            FANETAddressHelper();                                               //done
            FANETAddressHelper(Ipv4Address network, Ipv4Mask mask);             //done
            ~FANETAddressHelper();                                              //done

            void SetUp(Ipv4Address network, Ipv4Mask mask);                     //done
            void SetBases(std::vector<NetDeviceContainer> clustersDevices, std::vector<NetDeviceContainer> GDTtoCHLinksDevices);    //done
            static Ipv4Address GetBaseAddress(Ipv4Address ip);
        };
}

#endif