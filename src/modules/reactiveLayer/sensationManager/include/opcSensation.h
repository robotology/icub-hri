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

/**
 * \ingroup sensationManager
 */
class OpcSensation: public Sensation
{
private:
    icubclient::ICubClient *iCub;
    yarp::os::BufferedPort<yarp::os::Bottle> known_entities_port; //!< Output port to stream all known entities
    yarp::os::BufferedPort<yarp::os::Bottle> unknown_entities_port; //!< Output port to stream all unknown entities
    yarp::os::BufferedPort<yarp::os::Bottle> homeoPort; //!< Port to communicate with homeostasis
   
    /**
     * @brief fills an entity list with entities that follow a specific condition
     * @param list list to be added to
     * @param type entity type
     * @param name entity name
     */
    void addToEntityList(yarp::os::Bottle& list, std::string type, std::string name);

    /**
     * @brief handleEntities parses the OPC to identify entities following some condition, i.e. whether they are known or unknown
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
