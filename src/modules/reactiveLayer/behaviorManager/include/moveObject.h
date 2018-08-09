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
    MoveObject(yarp::os::Mutex* mut, yarp::os::ResourceFinder &rf, icubclient::ICubClient* iCub, std::string behaviorName):
        Behavior(mut, rf, iCub, behaviorName),
        target_pullback(-0.25),
        target_pushfront(-0.5),
        target_pushleft(-0.2),
        target_pushright(0.2)
    {
        ;
    }

    void configure();
};

#endif // MOVEOBJECT
