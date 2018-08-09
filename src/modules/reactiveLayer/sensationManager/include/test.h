#ifndef HOMEO_TEST
#define HOMEO_TEST

#include <string>
#include <iostream>
#include <iomanip>
#include <map>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

#include "icubclient/clients/icubClient.h"

#include "sensation.h"

/**
 * \ingroup sensationManager
 */
class TestSensation: public Sensation
{
private:
    bool on;
    string moduleName, unknown_obj_port_name, confusion_port_name;
    yarp::os::BufferedPort<Bottle> in;
    yarp::os::BufferedPort<Bottle> out;


public:
    TestSensation() : on(false) {
        ;
    }

    void configure();
    void publish();
    void close_ports() {
        in.interrupt();
        in.close();
        out.interrupt();
        out.close();
    }
};

#endif
