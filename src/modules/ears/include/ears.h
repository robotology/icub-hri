#ifndef EARS_H
#define EARS_H

#include <yarp/os/RFModule.h>
#include <yarp/os/Port.h>

namespace icubhri {
 class ICubClient;
}

/**
 * \ingroup ears
 */
class ears : public yarp::os::RFModule {
protected:
    icubhri::ICubClient *iCub;
    double      period;
    yarp::os::Port        rpc; //!< Response port
    yarp::os::Port        portToBehavior; //!< Port to behaviorManager
    yarp::os::Port        portToSpeechRecognizer; //!< Port to speechRecognizer
    yarp::os::Mutex       m; //!< Mutex indicating whether ears is listening to speechRecognizer
    bool        bShouldListen; //!< Whether ears should listen to speechRecognizer. Can be set via RPC port.
    std::string MainGrammar; //!< Name of grammar to be loaded

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
