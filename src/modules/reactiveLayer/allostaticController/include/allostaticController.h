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


/**
 * \ingroup allostaticController
 */
enum OutCZ {UNDER, OVER};

/**
 * \ingroup allostaticController
 */
struct DriveOutCZ {
    string name;
    OutCZ level;

};

/**
 * \ingroup allostaticController
 */
enum DriveUpdateMode {SENSATION_ON, SENSATION_OFF};

/**
 * \ingroup allostaticController
 */
class AllostaticDrive
{
public:
    string name; //!< Name of the drive
    bool active, manualMode;
    Port *behaviorUnderPort; //!< Port to be used when drive falls below homeostatic level
    Port *behaviorOverPort; //!< Port to be used when drive hits upper threshold of homeostatic level
    Port *homeoPort; //!< Port to homeostasis module
    BufferedPort<Bottle> *inputSensationPort; //!< Port from sensationManager
    Bottle behaviorUnderCmd; //!< Command to be send to behaviorUnderPort when drive falls below homeostatic level
    Bottle behaviorOverCmd; //!< Command to be send to behaviorOverPort when drive hits upper threshold of homeostatic level
    Bottle sensationOnCmd, sensationOffCmd, beforeTriggerCmd, afterTriggerCmd;
   
    AllostaticDrive() {
        manualMode = true;
        behaviorUnderPort = nullptr;
        behaviorOverPort = nullptr;
        homeoPort = nullptr;
        inputSensationPort = nullptr;
        active = false;
    }

    /**
     * @brief Interrupt ports of the drive
     */
    bool interrupt_ports();

    /**
     * @brief Close ports of the drive
     */
    bool close_ports();

    Bottle update(DriveUpdateMode mode);
   
    /**
     * @brief triggers a behavior if needs are out of threshold
     * @param mode under or below threshold
     */
    void triggerBehavior(OutCZ mode);
};

/**
 * \ingroup allostaticController
 */
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


    /**
     * @brief configureAllostatic creates drives from a config file
     * @param rf drives and config parameters
     * @return bool success
     */
    void configureAllostatic(yarp::os::ResourceFinder &rf);

    /**
     * @brief openPorts opens ports for homeostasis-allostatiController communication
     * @param driveName name of the drive to configure
     * @return bool success
     */
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

    /**
     * @brief updateAllostatic Update the drives accordingly to the stimuli
     * @return bool success
     */
    bool updateAllostatic();

    /**
     * @brief Normalize normalize drive priorities
     * @param vec list of priorities for each drive
     * @return bool false if no priority, true otherwise
     */
    bool Normalize(vector<double>& vec);

    /**
     * @brief chooseDrive Choose a drive out of CZ, according to drive priorities
     * @return bool success
     */
    DriveOutCZ chooseDrive();

    bool respond(const Bottle& cmd, Bottle& reply);
};

#endif
