#ifndef SENSATION
#define SENSATION

#include <string>
#include <yarp/os/all.h>

using namespace std;
using namespace yarp::os;

/**
 * \ingroup sensationManager
 */
class Sensation : public BufferedPort<Bottle>
{
public:

    virtual void configure() = 0;
    /**
     * @brief send sensation data to port
     */
    virtual void publish() = 0;
    virtual void close_ports() = 0;

};

#endif
