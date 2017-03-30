#include <yarp/os/all.h>
#include "wrdac/subsystems/subSystem_babbling.h"

bool icubclient::SubSystem_babbling::connect() {
    if(yarp::os::Network::isConnected(portRPC.getName(), "/babbling/rpc")) {
        return true;
    } else {
        return yarp::os::Network::connect(portRPC.getName(), "/babbling/rpc");
    }
}

icubclient::SubSystem_babbling::SubSystem_babbling(const std::string &masterName) : SubSystem(masterName) {
    portRPC.open(("/" + m_masterName + "/babbling:rpc").c_str());
    m_type = SUBSYSTEM_BABBLING;
}

void icubclient::SubSystem_babbling::Close() {
    portRPC.interrupt();
    portRPC.close();
}

bool icubclient::SubSystem_babbling::babblingArm(std::string babblingLimb, double train_duration) { // need to be extended for several agents
    yarp::os::Bottle bReq, bResp;
    bReq.addString("babbling");
    bReq.addString(babblingLimb);
    //changethe train_duration if specified
    if(train_duration >= 0.0){
        yInfo() << "Babbling with specific train_duration: " << train_duration;
        bReq.addDouble(train_duration);
    }
    portRPC.write(bReq, bResp);

    return bResp.get(0).asBool();
}

bool icubclient::SubSystem_babbling::babbling(int jointNumber, std::string babblingLimb, double train_duration) { // need to be extended for several agents
    yarp::os::Bottle bReq, bResp;
    bReq.addString("babbling");
    bReq.addString("joint");
    bReq.addInt(jointNumber);
    bReq.addString(babblingLimb);
    if(train_duration >= 0.0){
        yInfo() << "Babbling with specific train_duration: " << train_duration;
        bReq.addDouble(train_duration);
    }
    portRPC.write(bReq, bResp);

    return bResp.get(0).asBool();
}
