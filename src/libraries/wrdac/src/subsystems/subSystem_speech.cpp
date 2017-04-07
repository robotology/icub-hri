#include <yarp/os/all.h>
#include "wrdac/subsystems/subSystem_speech.h"

icubclient::SubSystem_Speech::SubSystem_Speech(const std::string &masterName) :SubSystem(masterName)
{
    tts.open(("/" + m_masterName + "/tts:o").c_str());
    ttsRpc.open(("/" + m_masterName + "/tts:rpc").c_str());
    m_type = SUBSYSTEM_SPEECH;
}

bool icubclient::SubSystem_Speech::connect()
{
    if(!yarp::os::Network::isConnected("/iSpeak/emotions:o", "/icub/face/emotions/in")) {
        yarp::os::Network::connect("/iSpeak/emotions:o", "/icub/face/emotions/in");
    }
    bool connected = true;
    if(!yarp::os::Network::isConnected(tts.getName(), "/iSpeak")) {
        connected &= yarp::os::Network::connect(tts.getName(), "/iSpeak");
    }
    if(!yarp::os::Network::isConnected(ttsRpc.getName(), "/iSpeak/rpc")) {
        connected &= yarp::os::Network::connect(ttsRpc.getName(), "/iSpeak/rpc");
    }

    return connected;
}

void icubclient::SubSystem_Speech::TTS(const std::string &text, bool shouldWait) {
    if(text=="") {
        yWarning() << "[SubSystem_Speech] Text is empty, not going to say anything";
        return;
    }
    //Clean the input of underscores.
    std::string tmpText = text;
    replace_all(tmpText, "_", " ");
    yarp::os::Bottle txt; txt.addString(tmpText.c_str());
    tts.write(txt);
    //int words = countWordsInString(text);
    //double durationMn =  words / (double)m_speed;
    //double durationS = durationMn *60.0;
    //yarp::os::Time::delay(durationS);
    yarp::os::Bottle cmd, reply;
    cmd.addVocab(VOCAB('s', 't', 'a', 't'));
    std::string status = "speaking";
    bool speechStarted = false;

    while (shouldWait && (!speechStarted || status == "speaking"))
    {
        ttsRpc.write(cmd, reply);
        status = reply.get(0).asString();
        if (!speechStarted && status != "quiet")
        {
            speechStarted = true;
        }
        yarp::os::Time::delay(0.2);
    }
}

void icubclient::SubSystem_Speech::SetOptions(const std::string &custom) {
    if(custom!="iCub") {
        yarp::os::Bottle param;
        param.addString("set");
        param.addString("opt");
        param.addString(custom.c_str());
        ttsRpc.write(param);
    } else {
        yWarning() << "SetOptions called with none for iSpeak";
    }
}

bool icubclient::SubSystem_Speech::isSpeaking() {
    yarp::os::Bottle cmd, reply;
    cmd.addVocab(VOCAB('s', 't', 'a', 't'));
    ttsRpc.write(cmd, reply);
    return (reply.get(0).asString() != "quiet");
}

void icubclient::SubSystem_Speech::Close()
{
    tts.interrupt();
    tts.close();
    ttsRpc.interrupt();
    ttsRpc.close();
}
