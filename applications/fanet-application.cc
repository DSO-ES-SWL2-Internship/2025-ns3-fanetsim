#include "fanet-application.h"

namespace ns3 
{
    FANETApplication::FANETApplication()
        : m_socket(nullptr), m_destAddr(Ipv4Address("0.0.0.0")), m_port(8080) {}

    FANETApplication::~FANETApplication(){}
}