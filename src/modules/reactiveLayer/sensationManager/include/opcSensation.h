#include <string>
#include <iostream>
#include <iomanip>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include "icubclient/clients/icubClient.h"
#include <map>

#include "sensation.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace icubclient;

class OpcSensation: public Sensation
{
private:
    ICubClient *iCub;
    yarp::os::BufferedPort<Bottle> unknown_entities_port;
    yarp::os::BufferedPort<Bottle> homeoPort;
    yarp::os::BufferedPort<Bottle> known_entities_port;
    yarp::os::BufferedPort<Bottle> opc_has_agent_port;
    yarp::os::BufferedPort<Bottle> is_touched_port;
    void addToEntityList(yarp::os::Bottle& list, std::string type, std::string name);
    Bottle handleEntities(); 

public:

    Bottle u_entities, k_entities, up_entities, kp_entities, p_entities, o_positions;
    void configure();
    void publish();
    int get_property(string name, string property);
    

    void close_ports() {
        unknown_entities_port.interrupt();
        unknown_entities_port.close();
        homeoPort.interrupt();
        homeoPort.close();    
        known_entities_port.interrupt();
        known_entities_port.close(); 
        opc_has_agent_port.interrupt();
        opc_has_agent_port.close();
        is_touched_port.interrupt();
        is_touched_port.close();
        iCub->close();
        delete iCub;
    }
};
