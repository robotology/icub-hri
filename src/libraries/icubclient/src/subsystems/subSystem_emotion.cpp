#include <yarp/os/all.h>
#include <algorithm>
#include "icubclient/subsystems/subSystem_emotion.h"

bool icubclient::SubSystem_emotion::connect() {
    if(yarp::os::Network::isConnected(portRPC.getName(), "/icub/face/emotions/in")) {
        return true;
    } else {
        return yarp::os::Network::connect(portRPC.getName(), "/icub/face/emotions/in");
    }
}

icubclient::SubSystem_emotion::SubSystem_emotion(const std::string &masterName) : SubSystem(masterName) {
    portRPC.open(("/" + m_masterName + "/emotion:rpc").c_str());
    m_type = SUBSYSTEM_EMOTION;
}

void icubclient::SubSystem_emotion::Close() {
    portRPC.interrupt();
    portRPC.close();
}

bool icubclient::SubSystem_emotion::setEmotion(Emotion emotion) {
    return setEmotion(emotion, Part::all);
}

bool icubclient::SubSystem_emotion::setEmotion(Emotion emotion, Part part) {
    yarp::os::Bottle bReq, bResp;
    std::string spart, semotion;

    switch(emotion)
    {
        case Emotion::neutral    : semotion="neu";   break;
        case Emotion::talking    : semotion="tal";   break;
        case Emotion::happy      : semotion="hap";   break;
        case Emotion::sad        : semotion="sad";   break;
        case Emotion::surprised  : semotion="sur";   break;
        case Emotion::evil       : semotion="evi";   break;
        case Emotion::angry      : semotion="ang";   break;
        case Emotion::shy        : semotion="shy";   break;
        case Emotion::cunning    : semotion="cun";   break;
    }

    switch(part)
    {
        case Part::mouth          : spart="mou";   break;
        case Part::eyelids        : spart="eli";   break;
        case Part::left_eyebrow   : spart="leb";   break;
        case Part::right_eyebrow  : spart="reb";   break;
        case Part::all            : spart="all";   break;
    }

    bReq.addString("set");
    bReq.addString(spart);
    bReq.addString(semotion);
    portRPC.write(bReq, bResp);
    if (bResp.toString() == "ack")
    {
        return true;
    }
    else
    {
        yError()<<bResp.toString();
        return false;
    }

}
