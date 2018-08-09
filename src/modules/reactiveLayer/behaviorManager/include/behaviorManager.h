#ifndef BEHAVIORMANAGER_H
#define BEHAVIORMANAGER_H

#include <iostream>
#include <yarp/os/all.h>
#include "behavior.h"


/**
 * \ingroup behaviorManager
 */
class BehaviorManager: public yarp::os::RFModule
{
private:
    std::vector<Behavior*> behaviors; //!< List of all behaviors

    std::string moduleName;

    double period;

    yarp::os::Port rpc_in_port;

    icubclient::ICubClient *iCub;

    yarp::os::Mutex mut;

public:
    BehaviorManager() : period(1.0), iCub(nullptr) {
        ;
    }

    bool configure(yarp::os::ResourceFinder &rf);

    bool interruptModule();

    bool close();

    double getPeriod()
    {
        return period;
    }

    bool updateModule();

    //RPC & scenarios
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);

};

#endif
