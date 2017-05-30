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
   
    /**
     * @brief fills an entity list with entities that follow a specific condition
     * @param list list to be added to
     * @param type entity type
     * @param name entity name
     * @return void
     */
    void addToEntityList(yarp::os::Bottle& list, std::string type, std::string name);
    /**
     * @brief handleEntities parses the OPC to identify entities following some condition
     * @param none
     * @return void
     */
    yarp::os::Bottle handleEntities();

public:

    yarp::os::Bottle u_entities, k_entities, up_entities, kp_entities, p_entities, o_positions;
    void configure();
    void publish();

    /**
     * @brief get_property quickly answers a boolean query
     * @param name entity name
     * @param property entity property (e.g. present or known)
     * @return int 1 if entity has the given property
     */
    int get_property(std::string name, std::string property);

    void close_ports();
};

#endif
