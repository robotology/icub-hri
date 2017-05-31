#ifndef MOVEOBJECT
#define MOVEOBJECT

#include "behavior.h"

/**
 * \ingroup behaviorManager
 */
class MoveObject: public Behavior
{
protected:
    void run(const yarp::os::Bottle &args);

    double target_pullback;
    double target_pushfront;
    double target_pushleft;
    double target_pushright;

    void close_extra_ports() {
        ;
    }

public:
    MoveObject(yarp::os::Mutex* mut, yarp::os::ResourceFinder &rf, icubhri::ICubClient* iCub, std::string behaviorName): Behavior(mut, rf, iCub, behaviorName) {
        ;
    }

    void configure();
};

#endif // MOVEOBJECT
