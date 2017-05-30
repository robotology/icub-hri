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

    /**
     * @brief addNewDrive add new  drive with default dynamics
     * @param  driveName name of the drive
     * @return bool success
     */
    bool addNewDrive(string driveName);

    /**
     * @brief addNewDrive add new drive from bottle
     * @param  grpHomeostatic bottle from config file or port
     * @return bool success
     */
    bool addNewDrive(string driveName, yarp::os::Bottle& grpHomeostatic);
   
    /**
     * @brief removeDrive removes drive
     * @param  d ID number of the drive
     * @return bool success
     */
    bool removeDrive(int d);
    bool verbose;

public:

    Drive bDrive(yarp::os::Bottle* b)
    {
        Drive d = Drive(b->get(0).asString(),b->get(1).asDouble(),b->get(2).asString(),b->get(3).asDouble(),b->get(4).asDouble(),b->get(5).asDouble(),b->get(6).asBool());
        return d;
    }

    /**
     * @brief openPorts opens default ports for external communication
     * @param  driveName name of the drive
     * @return int success
     */
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

    //RPC & scenarios
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);
};

#endif
