#include <functional>

#include "icubclient/clients/icubClient.h"

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
    std::vector<double> spd1;
    std::vector<double> spd2;
    bool move;
    int iter;

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
    bool    populateMoving();
    bool    populateSpecific();
    bool    populateSpecific1(const yarp::os::Bottle &bInput);
    bool    populateSpecific2();
    bool    populateSpecific3();

    bool    addUnknownEntity(const yarp::os::Bottle &bInput);
    bool    setAttributeEntity(const yarp::os::Bottle &bInput, std::function<void(icubclient::Object*, double)> f_setter);

    //RPC & scenarios
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);
};
