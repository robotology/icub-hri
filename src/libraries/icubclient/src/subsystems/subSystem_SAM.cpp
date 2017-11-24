#include <tuple>
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

bool icubclient::SubSystem_SAM::attentionModulation(const std::string &mode) {
    if (mode == "stop" || mode == "continue")
    {
        yarp::os::Bottle bReq, bResp;
        bReq.addString("attentionModulation " + mode);
        portRPC.write(bReq, bResp);
        if (bResp.toString() == "ack")
        {   
            if (mode == "stop")
            {
                attentionSAM = false;
                yInfo()<<"Received ack. Attention off";
            }
            else if(mode == "continue")
            {
                attentionSAM = true;
                yInfo()<<"Received ack. Attention on";
            }
            return true;
        }
        else
        {
            yError()<<bResp.toString();
            return false;
        }
    }
    else
    {
        yError()<<"mode can be 'stop' or 'continue'. Received: "+ mode;
        return false;
    }
}

yarp::os::Bottle icubclient::SubSystem_SAM::askXLabel(const std::string &model) {
    yarp::os::Bottle bReq, bResp, bRet;
    bReq.addString("ask_" + model + "_label");
    portRPC.write(bReq, bResp);
    if (bResp.get(0).toString() == "ack")
    {   
        std::string classification = bResp.get(1).toString();
        yInfo()<<model + " Classification = " + classification;
        bRet.addString("true");
        bRet.addString(classification);
        return bRet;
    }
    else
    {
        yError()<<bResp.toString();
        bRet.addString("false");
        bRet.addString("nack");
        return bRet;
    }
}
