#ifndef HOMEO_MANAGER_ICUB
#define HOMEO_MANAGER_ICUB

#include <string>
#include <iostream>
#include <iomanip>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/SVD.h>
#include <map>
#include <vector>

#include "homeostasisManager.h"

class HomeostaticModule: public yarp::os::RFModule
{
private:

    std::string moduleName;
    double period;


    std::vector< yarp::os::BufferedPort<yarp::os::Bottle> * > input_ports;
    std::vector< yarp::os::BufferedPort<yarp::os::Bottle> * > outputM_ports;
    std::vector< yarp::os::BufferedPort<yarp::os::Bottle> * > outputm_ports;
    HomeostasisManager* manager;
    yarp::os::BufferedPort<yarp::os::Bottle> input_port;
    yarp::os::Port    rpc;

    bool addNewDrive(string driveName, yarp::os::Bottle& grpHomeostatic);
    bool addNewDrive(string driveName);
    bool removeDrive(int d);
    bool verbose;

public:

    Drive bDrive(yarp::os::Bottle* b)
    {
        Drive d = Drive(b->get(0).asString(),b->get(1).asDouble(),b->get(2).asString(),b->get(3).asDouble(),b->get(4).asDouble(),b->get(5).asDouble(),b->get(6).asBool());
        return d;
    }

    int openPorts(std::string driveName);

    bool configure(yarp::os::ResourceFinder &rf);

    bool interruptModule()
    {
        return true;
    }

    bool close();

    double getPeriod()
    {
        return period;
    }
    bool updateModule();

    bool updateAllostatic();

    bool processAvoidance(int d, yarp::os::Bottle* avoidanceBottle);

    //RPC & scenarios
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);
};

#endif
