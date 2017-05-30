#ifndef BEHAVIOR
#define BEHAVIOR

#include <string>
#include <yarp/os/all.h>
#include "icubclient/clients/opcClient.h"
#include "icubclient/clients/icubClient.h"
#include "icubclient/subsystems/subSystem_SAM.h"

class Behavior
{
private:
    yarp::os::Mutex* mut;
    yarp::os::BufferedPort<yarp::os::Bottle> behavior_start_stop_port;

protected:
    icubclient::ICubClient *iCub;
    yarp::os::ResourceFinder& rf;
    virtual void run(const yarp::os::Bottle &args) = 0;
    virtual void close_extra_ports() = 0;

public:
    yarp::os::Port rpc_out_port;
    std::string behaviorName, from_sensation_port_name, external_port_name;
    yarp::os::BufferedPort<yarp::os::Bottle> sensation_port_in;

    Behavior(yarp::os::Mutex* _mut, yarp::os::ResourceFinder &_rf, icubclient::ICubClient* _iCub, std::string _behaviorName) : mut(_mut), iCub(_iCub), rf(_rf), behaviorName(_behaviorName){
        from_sensation_port_name = "None";
        external_port_name = "None";
    }
    virtual ~Behavior() {}

    /**
     * @brief openPorts open ports defined for a behavior
     * @param port_name_prefix unique prefix name for the port
     * @return void
     */
    void openPorts(std::string port_name_prefix) {
        if (from_sensation_port_name != "None") {
            sensation_port_in.open("/" + port_name_prefix +"/" + behaviorName + "/sensation:i");
        }
        if (external_port_name != "None") {
            rpc_out_port.open("/" + port_name_prefix +"/" + behaviorName + "/to_external_module");
        }
        behavior_start_stop_port.open("/" + port_name_prefix +"/" + behaviorName + "/start_stop:o");
    }
   
    /**
     * @brief trigger triggers this behavior and blocks the mutex
     * @param args bottle with arguments for this behaviours if not provided, taken from sensations or return error
     * @return bool false if not triggered because of mutex blocked
     */
    bool trigger(const yarp::os::Bottle& args) {
        yDebug() << behaviorName << "::trigger starts";
        if (mut->tryLock()) {
            yDebug() << behaviorName << "::trigger mutex locked";
            yarp::os::Bottle & msg = behavior_start_stop_port.prepare();
            msg.clear();
            msg.addString("start");
            behavior_start_stop_port.write();

            run(args);

            msg = behavior_start_stop_port.prepare();
            msg.clear();
            msg.addString("stop");
            behavior_start_stop_port.write();
            mut->unlock();
            yDebug() << behaviorName << "::trigger mutex unlocked";
            yDebug() << behaviorName << "::trigger ends";
            return true;
        }
        yDebug() << behaviorName << " not executed, mutex was closed";
        yDebug() << behaviorName << "::trigger ends";
        return false;
    }

    virtual void configure() = 0;

    void interrupt_ports() {
        sensation_port_in.interrupt();
        rpc_out_port.interrupt();
        behavior_start_stop_port.interrupt();
    }

    void close_ports() {
        close_extra_ports();
        sensation_port_in.interrupt();
        sensation_port_in.close();
        rpc_out_port.interrupt();
        rpc_out_port.close();
        behavior_start_stop_port.interrupt();
        behavior_start_stop_port.close();
    }
   
};

#endif
