#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"

using namespace ns3;

class AddServer : public Application {
    private:
        Ptr<Socket> m_socket;

        void HandleRead(Ptr<Socket> socket);


    public:
        AddServer() {}

        void StartApplication() override;

        void StopApplication() override;
};
