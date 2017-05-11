#include "icubclient/clients/icubClient.h"

using namespace std;
using namespace yarp::os;
using namespace icubclient;

class ears : public RFModule {
protected:
    ICubClient *iCub;
    double      period;
    Port        rpc;
    Port        portToBehavior;
    Port        portToSpeechRecognizer;
    Mutex       m;
    bool        bShouldListen;
    std::string MainGrammar;

public:
    bool configure(yarp::os::ResourceFinder &rf);

    bool interruptModule();

    bool close();

    double getPeriod()
    {
        return period;
    }

    bool    updateModule();

    //RPC & scenarios
    bool respond(const Bottle& cmd, Bottle& reply);
};
