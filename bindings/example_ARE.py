import yarp
import icubclient
import sys


if __name__ == '__main__':
    yarp.Network.init()

    logger = yarp.Log()
    iCub = icubclient.ICubClient("AREiCubClientExample", "icubClient", "example_ARE.ini")

    rfClient = yarp.ResourceFinder()
    rfClient.setVerbose(True)
    rfClient.setDefaultContext("icubClient")
    rfClient.setDefaultConfigFile("example_ARE.ini")
    rfClient.configure(sys.argv)

    # we connect to both ARE and OPC
    if not iCub.connect():
        logger.error() << "[ARE_KARMAiCubClientExample] ARE and/or OPC seems unavailabe!"

    if rfClient.check("target"):
        target = rfClient.find("target").asString()
        logger.info() << "target name set to %s"
        logger.info() << target
    else:
        target = "octopus"
        logger.info() << "target name set to default, i.e. "
        logger.info() << target

    radius = 0.025

    # object location in the iCub frame Vector
    x = yarp.Vector(3)
    x[0] = -0.35
    x[1] = -0.05
    x[2] = -0.05

    iCub.home()
    logger.info() << "pointing at the object ... "
    options = yarp.Bottle("right") # force the use of the right hand
    iCub.point(x, options) # automatic selection of the hand
    yarp.Time.delay(2.0)

    iCub.home()
    logger.info() << "try to grasp "
    logger.info() << target
    ok = iCub.take(target, options)
    if ok:
        logger.info() << "grasped"
    else:
        logger.info() << "missed"
    iCub.home()

    if ok:
        x[2] += 1.2 * radius
        logger.info() << "releasing ... "
        iCub.release(x)
        iCub.home()

    logger.info() << "AREiCubClientExample shutting down ... "
    iCub.close()
    yarp.Network.fini()