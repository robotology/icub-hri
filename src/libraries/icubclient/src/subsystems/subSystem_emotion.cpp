#include <yarp/os/all.h>
#include <algorithm>
#include <map>
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

bool icubclient::SubSystem_emotion::setEmotion(std::string emotion) {
    return setEmotion(emotion, "all");
}

bool icubclient::SubSystem_emotion::setEmotion(std::string emotion, std::string part) {
    std::pair<std::string, std::string> emPair;

    if(mapToEmotion.find(emotion) != mapToEmotion.end() &&
       mapToPart.find(part)       != mapToPart.end())
    {
        emPair.first = mapToEmotion.find(emotion)->second;
        emPair.second = mapToPart.find(part)->second;

        return sendEmotion(emPair);
    }
    else if(mapToEmotion.find(emotion) == mapToEmotion.end())
    {
        yError() << "Emotion " << emotion << " not found";
        return false;
    }
    else if(mapToPart.find(part) == mapToPart.end())
    {
        yError() << "Part " << part << " not found";
        return false;
    }
    else
    {
        yError() << "Uncaught error";
        return false;
    }
}

bool ool icubclient::SubSystem_emotion::sendEmotion(std::pair<std::string, std::string> emPair) {
    yarp::os::Bottle bReq, bResp;

    bReq.addString("set");
    bReq.addString(emPair.second);
    bReq.addString(emPair.first);

    yInfo()<<"Sending " << bReq.toString();

    portRPC.write(bReq, bResp);
    if (bResp.toString() == "ok")
    {
        currentEmotionPair = emPair
        return true;
    }
    else
    {
        yError()<<bResp.toString();
        return false;
    }
}

std::pair<std::string, std::string> icubclient::SubSystem_emotion::getEmotion()
{
    return currentEmotionPair;
}
