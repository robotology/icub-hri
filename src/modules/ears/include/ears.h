#ifndef EARS_H
#define EARS_H

#include <yarp/os/RFModule.h>
#include <yarp/os/Port.h>

namespace icubclient {
 class ICubClient;
}

class ears : public yarp::os::RFModule {
protected:
    icubclient::ICubClient *iCub;
    double      period;
    yarp::os::Port        rpc;
    yarp::os::Port        portToBehavior;
    yarp::os::Port        portToSpeechRecognizer;
    yarp::os::Mutex       m;
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
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);
};

#endif
