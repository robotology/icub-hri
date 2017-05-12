#include <functional>

#include <yarp/os/all.h>

namespace icubclient {
    class ICubClient;
    class Object;
}

class opcPopulater : public yarp::os::RFModule {
protected:

    icubclient::ICubClient *iCub;
    double period;
    yarp::os::Port rpc;

    double X_obj;
    double Y_obj;
    double Z_obj;
    double X_ag;
    double Y_ag;
    double Z_ag;
    double noise;

public:
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
    bool    populateEntityRandom(const yarp::os::Bottle &bInput);
    bool    populateRedBall();
    bool    populateSpecific();
    bool    populateTwoUnknowns();

    bool    addUnknownEntity(const yarp::os::Bottle &bInput);
    bool    setAttributeEntity(const yarp::os::Bottle &bInput, std::function<void(icubclient::Object*, double)> f_setter);

    //RPC & scenarios
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);
};
