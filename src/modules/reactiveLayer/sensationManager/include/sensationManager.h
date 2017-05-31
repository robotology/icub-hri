#ifndef SENSATION_MANAGER
#define SENSATION_MANAGER

#include <string>
#include <iostream>
#include <iomanip>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <map>

#include "opcSensation.h"
#include "test.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace icubhri;

/**
 * \ingroup sensationManager
 */
class SensationManager: public RFModule
{
private:
    vector<Sensation*> sensations;
    Bottle sensationList;
    string moduleName;
    Port rpc_in_port;

    double period;

public:
    bool configure(yarp::os::ResourceFinder &rf);

    bool interruptModule()
    {
        return true;
    }

    double getPeriod()
    {
        return period;
    }

    bool updateModule();
    bool respond(const Bottle& cmd, Bottle& reply);

    bool close();

};

#endif
