#include <functional>

#include <yarp/os/all.h>

namespace icubclient {
    class ICubClient;
    class Object;
}

/**
 * \ingroup opcPopulator
 */
class opcPopulator : public yarp::os::RFModule {
protected:

    icubclient::ICubClient *iCub;
    double period;
    yarp::os::Port rpc;

    double X_obj; //!< X coordinate of object used within populateSpecific() for bottom_left and bottom_right
    double Y_obj; //!< Y coordinate of object used within populateSpecific() for bottom_left (inverted) and bottom_right
    double Z_obj; //!< Z coordinate of object used within populateSpecific() for bottom_left and bottom_right
    double X_ag; //!< X coordinate of object used within populateSpecific() for top_left and top_right
    double Y_ag; //!< Y coordinate of object used within populateSpecific() for top_left (inverted) and top_right
    double Z_ag; //!< Z coordinate of object used within populateSpecific() for top_left and top_right
    double noise; //!< (Gaussian) noise to be added to X_obj, Y_obj, Z_obj, X_ag, Y_ag, Z_ag in populateSpecific()

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
    /**
     * @brief populateEntityRandom populate entity from Bottle
     * @param bInput Bottle containing type "populateEntityRandom type name" where type is "agent" or "object" and name is the name of the new entity
     * @return true if successful
     */
    bool populateEntityRandom(const yarp::os::Bottle &bInput);

    /**
     * @brief populateRedBall populate object with name "red_ball" at location (-0.4, 0.25, 0.0); is_present=0.0; color=(250,0,0)
     * @return true if successful
     */
    bool populateRedBall();

    /**
     * @brief populateSpecific populate four objects ("bottom_left", "bottom_right", according to X_obj, Y_obj, Z_obj, and "top_left", "top_right" according to X_ag, Y_ag, Z_ag
     * @return true if successful
     */
    bool populateSpecific();

    /**
     * @brief populateTwoUnknowns populate two objects: "unknown_1" at (-0.4, 0.25, 0.0) and "unknown_2" at (-0.4, -0.25, 0.0), both present and randomly colored
     * @return true if successful
     */
    bool populateTwoUnknowns();

    /**
     * @brief addUnknownEntity populate object / agent at pseudo-random location, pseudo-randomly colored
     * @param bInput "addUnknownEntity type" where type is "agent" or "object"
     * @return true if successful
     */
    bool addUnknownEntity(const yarp::os::Bottle &bInput);

    /**
     * @brief setAttributeEntity set an attribute of an object
     * @param bInput Bottle with format "setAttributeEntity name targetValue" where name defines the object and targetValue defines the new value
     * @param f_setter function pointer to function to be called, e.g. to icubclient::Object::setSaliency()
     * @return true if successful
     */
    bool setAttributeEntity(const yarp::os::Bottle &bInput, std::function<void(icubclient::Object*, double)> f_setter);

    //RPC & scenarios
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);
};
