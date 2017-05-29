#ifndef OPCSENSATION
#define OPCSENSATION

#include <string>
#include <iostream>
#include <iomanip>
#include <map>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

#include "sensation.h"

namespace icubclient {
    class ICubClient;
}

class OpcSensation: public Sensation
{
private:
    icubclient::ICubClient *iCub;
    yarp::os::BufferedPort<yarp::os::Bottle> unknown_entities_port;
    yarp::os::BufferedPort<yarp::os::Bottle> homeoPort;
    yarp::os::BufferedPort<yarp::os::Bottle> known_entities_port;
    yarp::os::BufferedPort<yarp::os::Bottle> opc_has_agent_port;
    void addToEntityList(yarp::os::Bottle& list, std::string type, std::string name);
    yarp::os::Bottle handleEntities();

public:

    yarp::os::Bottle u_entities, k_entities, up_entities, kp_entities, p_entities, o_positions;
    void configure();
    void publish();
    int get_property(std::string name, std::string property);

    void close_ports();
};

#endif
