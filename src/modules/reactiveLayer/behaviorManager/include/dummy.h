#ifndef DUMMY_H
#define DUMMY_H

#include <iostream>
#include <yarp/os/all.h>

#include "behavior.h"

/**
 * \ingroup behaviorManager
 */
class Dummy: public Behavior
{
private:
    static int n_instances;
    int id;

protected:
    void run(const yarp::os::Bottle &/*args*/) {
        yDebug() << "Dummy::run start " + behaviorName;
        yarp::os::Time::delay(4);
        yDebug() << "Dummy::run stop " + behaviorName;
    }

    void close_extra_ports() {
        ;
    }

public:
    Dummy(yarp::os::Mutex* mut, yarp::os::ResourceFinder &rf, icubhri::ICubClient* iCub, std::string behaviorName): Behavior(mut, rf, iCub, behaviorName) {
        n_instances++;
        id = n_instances;
    }

    void configure() {
        ;
    }
};

#endif
