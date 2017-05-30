#ifndef TAGGING_H
#define TAGGING_H

#include <iostream>
#include <yarp/os/all.h>

#include "behavior.h"


/**
 * \ingroup behaviorManager
 */
class Tagging: public Behavior
{
protected:
    void run(const yarp::os::Bottle &args);
    void close_extra_ports() {
        ;
    }

public:
    Tagging(yarp::os::Mutex* mut, yarp::os::ResourceFinder &rf, icubclient::ICubClient* iCub, std::string behaviorName): Behavior(mut, rf, iCub, behaviorName) {
        ;
    }
       
    void configure();
};

#endif
