#ifndef POINTING_H
#define POINTING_H

#include <iostream>
#include <yarp/os/all.h>

#include "behavior.h"


class Pointing: public Behavior
{
protected:
    void run(const yarp::os::Bottle &args);

    void close_extra_ports() {
        ;
    }

public:
    Pointing(yarp::os::Mutex* mut, yarp::os::ResourceFinder &rf, icubclient::ICubClient* iCub, std::string behaviorName): Behavior(mut, rf, iCub, behaviorName) {
        ;
    }
        
    void configure();
};

#endif
