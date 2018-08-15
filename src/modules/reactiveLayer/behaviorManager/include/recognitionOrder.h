#ifndef RECOGNITIONORDER_H
#define RECOGNITIONORDER_H

#include <iostream>
#include <yarp/os/all.h>

#include "behavior.h"

/**
 * \ingroup behaviorManager
 */
class RecognitionOrder: public Behavior
{
private:
    yarp::os::Port port_to_homeo;
    std::string port_to_homeo_name;
    std::string homeoPort;

protected:
    void run(const yarp::os::Bottle &args);
    void close_extra_ports() {
        port_to_homeo.interrupt();
        port_to_homeo.close();
    }

public:
    RecognitionOrder(yarp::os::Mutex* mut, yarp::os::ResourceFinder &rf, icubclient::ICubClient* iCub, std::string behaviorName): Behavior(mut, rf, iCub, behaviorName), manual(true) {
        ;
    }

    void configure();
    bool manual;
};

#endif
