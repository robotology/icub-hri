#include <yarp/os/all.h>
#include "wrdac/subsystems/subSystem_speech.h"

wysiwyd::wrdac::SubSystem_Speech::SubSystem_Speech(const std::string &masterName) :SubSystem(masterName)
{
    tts.open(("/" + m_masterName + "/tts:o").c_str());
    ttsRpc.open(("/" + m_masterName + "/tts:rpc").c_str());
    stt.open(("/" + m_masterName + "/stt:i").c_str());
    sttRpc.open(("/" + m_masterName + "/stt:rpc").c_str());
    m_type = SUBSYSTEM_SPEECH;
    opc = new OPCClient(m_masterName+"/opc_from_speech");
}

bool wysiwyd::wrdac::SubSystem_Speech::connect()
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
    if(!yarp::os::Network::isConnected("/speechRecognizer/recog/continuousGrammar:o", stt.getName().c_str())) {
        connected &= yarp::os::Network::connect("/speechRecognizer/recog/continuousGrammar:o", stt.getName().c_str());
    }
    if(!yarp::os::Network::isConnected(sttRpc.getName().c_str(), "/speechRecognizer/rpc")) {
        connected &= yarp::os::Network::connect(sttRpc.getName().c_str(), "/speechRecognizer/rpc");
    }

    opc->connect("OPC");

    return connected;
}

unsigned int wysiwyd::wrdac::SubSystem_Speech::countWordsInString(const std::string &str)
{
    std::stringstream stream(str);
    return std::distance(std::istream_iterator<std::string>(stream), std::istream_iterator<std::string>());
}

void wysiwyd::wrdac::SubSystem_Speech::TTS(const std::string &text, bool shouldWait, bool recordABM, std::string addressee) {
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

yarp::os::Bottle *wysiwyd::wrdac::SubSystem_Speech::STT(const std::string &grammar, double timeout)
{
    //todo
    return NULL;
}

yarp::os::Bottle *wysiwyd::wrdac::SubSystem_Speech::STT(bool isBlocking)
{
    return stt.read(isBlocking);
}

void wysiwyd::wrdac::SubSystem_Speech::STTflush()
{
    int pendingReads = stt.getPendingReads();
    if (pendingReads > 0)
        std::cout << "[subsystem.speech] Flushing " << pendingReads << " pending read" << std::endl;

    for (int i = 0; i < pendingReads; i++)
        stt.read(false);
}

void wysiwyd::wrdac::SubSystem_Speech::STT_ExpandVocabulory(const std::string &vocabuloryName, const std::string &word)
{
    yarp::os::Bottle bAugmentVocab;
    bAugmentVocab.addString("rgm");
    bAugmentVocab.addString("vocabulory");
    bAugmentVocab.addString("add");
    std::string sVocabTemp = "#";
    sVocabTemp += vocabuloryName.c_str();
    bAugmentVocab.addString(sVocabTemp.c_str());
    bAugmentVocab.addString(word.c_str());
    sttRpc.write(bAugmentVocab);
}

void wysiwyd::wrdac::SubSystem_Speech::SetOptions(const std::string &custom) {
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

bool wysiwyd::wrdac::SubSystem_Speech::isSpeaking() {
    yarp::os::Bottle cmd, reply;
    cmd.addVocab(VOCAB('s', 't', 'a', 't'));
    ttsRpc.write(cmd, reply);
    return (reply.get(0).asString() != "quiet");
}

void wysiwyd::wrdac::SubSystem_Speech::Close()
{
    tts.interrupt();
    tts.close();
    ttsRpc.interrupt();
    ttsRpc.close();
    stt.interrupt();
    stt.close();
    sttRpc.interrupt();
    sttRpc.close();
    opc->interrupt();
    opc->close();

    delete opc;
}
