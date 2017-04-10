#include <yarp/os/all.h>
#include "icubclient/subsystems/subSystem_agentDetector.h"

bool icubclient::SubSystem_agentDetector::connect() {
    if(yarp::os::Network::isConnected(portRPC.getName(), "/agentDetector/rpc")) {
        return true;
    } else {
        return yarp::os::Network::connect(portRPC.getName(), "/agentDetector/rpc");
    }
}

icubclient::SubSystem_agentDetector::SubSystem_agentDetector(const std::string &masterName) : SubSystem(masterName) {
    portRPC.open(("/" + m_masterName + "/agentDetector:rpc").c_str());
    m_type = SUBSYSTEM_AGENTDETECTOR;
}

void icubclient::SubSystem_agentDetector::Close() {
    portRPC.interrupt();
    portRPC.close();
}

bool icubclient::SubSystem_agentDetector::changeDefaultName(std::string new_name) { // need to be extended for several agents
    yarp::os::Bottle bReq, bResp;
    bReq.addString("change_partner_name");
    bReq.addString(new_name);
    portRPC.write(bReq, bResp);

    if(bResp.get(0).asString()=="ack")
        return true;
    else
        return false;
}

void icubclient::SubSystem_agentDetector::pause() {
    yarp::os::Bottle bReq, bResp;
    bReq.addString("pause");
    portRPC.write(bReq, bResp);
}

void icubclient::SubSystem_agentDetector::resume() {
    yarp::os::Bottle bReq, bResp;
    bReq.addString("resume");
    portRPC.write(bReq, bResp);
}
