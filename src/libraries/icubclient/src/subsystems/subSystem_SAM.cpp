#include <yarp/os/all.h>
#include "icubclient/subsystems/subSystem_SAM.h"

bool icubclient::SubSystem_SAM::connect() {
    if(yarp::os::Network::isConnected(portRPC.getName(), "/sam/rpc:i")) {
        return true;
    } else {
        return yarp::os::Network::connect(portRPC.getName(), "/sam/rpc:i");
    }
}

icubclient::SubSystem_SAM::SubSystem_SAM(const std::string &masterName) : SubSystem(masterName) {
    portRPC.open(("/" + m_masterName + "/sam:rpc").c_str());
    m_type = SUBSYSTEM_SAM;
}

void icubclient::SubSystem_SAM::Close() {
    portRPC.interrupt();
    portRPC.close();
}

void icubclient::SubSystem_SAM::pause() {
    yarp::os::Bottle bReq, bResp;
    bReq.addString("pause");
    portRPC.write(bReq, bResp);
}

void icubclient::SubSystem_SAM::resume() {
    yarp::os::Bottle bReq, bResp;
    bReq.addString("resume");
    portRPC.write(bReq, bResp);
}
