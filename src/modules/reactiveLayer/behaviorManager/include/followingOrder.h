#ifndef FOLLOWINGORDER_H
#define FOLLOWINGORDER_H

#include <iostream>
#include <yarp/os/all.h>

#include "behavior.h"


class FollowingOrder: public Behavior
{
private:
    yarp::os::Port port_to_homeo;
    std::string port_to_homeo_name;

    void run(const yarp::os::Bottle &args);
    std::string homeoPort;

    //followingOrder option
    std::string babblingArm;

public:
    FollowingOrder(yarp::os::Mutex* mut, yarp::os::ResourceFinder &rf, std::string behaviorName): Behavior(mut, rf, behaviorName) {
        ;
    }
      
    void configure();
    bool manual;

    bool handleAction(std::string type, std::string target, std::string action);
    bool handleActionBP(std::string type, std::string target, std::string action);
    bool handleSearch(std::string type, std::string action, bool verboseSearch);
    bool handleNarrate();
    bool handleGame(std::string type);
    bool handleEnd();

    void close_extra_ports() {
        port_to_homeo.interrupt();
        port_to_homeo.close();
    }
};

#endif
