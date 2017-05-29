#ifndef ALLOSTATICCONTROLLER_H
#define ALLOSTATICCONTROLLER_H

#include <string>
#include <iostream>
#include <iomanip>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Rand.h>
#include <map>
#include "icubclient/clients/icubClient.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::math;
using namespace icubclient;

enum OutCZ {UNDER, OVER};

struct DriveOutCZ {
    string name;
    OutCZ level;

};

enum DriveUpdateMode {SENSATION_ON, SENSATION_OFF};

class AllostaticDrive
{
public: 
    string name;
    bool active, manualMode;
    Port *behaviorUnderPort;
    Port *behaviorOverPort;
    Port *homeoPort;
    BufferedPort<Bottle> *inputSensationPort;
    Bottle behaviorUnderCmd;
    Bottle behaviorOverCmd;
    Bottle sensationOnCmd, sensationOffCmd, beforeTriggerCmd, afterTriggerCmd;
    
    AllostaticDrive() {
        manualMode = true;
        behaviorUnderPort = nullptr;
        behaviorOverPort = nullptr;
        homeoPort = nullptr;
        inputSensationPort = nullptr;
        active = false;
    }

    bool interrupt_ports();

    bool close_ports();

    Bottle update(DriveUpdateMode mode);

    void triggerBehavior(OutCZ mode);
};

class AllostaticController: public RFModule
{
private:
    Bottle drivesList;
    
    Port to_homeo_rpc, rpc_in_port, to_behavior_rpc;
    string moduleName;
    string homeo_name;

    double period;

    map<string, AllostaticDrive> allostaticDrives;


    vector<double> drivePriorities;

    vector< yarp::os::BufferedPort<Bottle>* > outputM_ports;
    vector< yarp::os::BufferedPort<Bottle>* > outputm_ports;


    //Configuration
    void configureAllostatic(yarp::os::ResourceFinder &rf);

    bool openPorts(string driveName);
public:
    bool configure(yarp::os::ResourceFinder &rf);
    bool interruptModule();

    bool close();

    double getPeriod()
    {
        return period;
    }

    bool updateModule();

    //Update the drives accordingly to the stimuli
    bool updateAllostatic();

    bool Normalize(vector<double>& vec);

    // Choose a drive out of CZ, according to drive priorities
    DriveOutCZ chooseDrive();

    bool respond(const Bottle& cmd, Bottle& reply);
};

#endif
