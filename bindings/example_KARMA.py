import yarp
import icubhri


if __name__ == '__main__':
    yarp.Network.init()

    logger = yarp.Log()
    iCub = icubhri.ICubClient("KARMAiCubClientExample", "icubClient", "example_ARE_KARMA.ini")

    # we connect to ARE and KARMA, which is defined in "example_ARE_KARMA.ini"
    if not iCub.connectSubSystems():
        logger.error("KARMA seems unavailabe!")

    # object location in the iCub frame Vector
    x = yarp.Vector(3)
    x[0] = -0.30
    x[1] = 0.05
    x[2] = -0.05

    # push left with pure API of KARMA.Please see KARMA document for details of how to define arguments
    iCub.home() # Home by using ARE
    x[1] = x[1] - 0.1
    logger.info() << "try to push left with KARMA..."
    if iCub.pushKarma(x, 180, 0.2): # Object will be push to left with y-coordinate as -0.05 from -0.25
        logger.info() << "success"
    else:
        logger.info() << "fail"
    yarp.Time.delay(4.0)

    # push right with pure API of KARMA.Please see KARMA document for details of how to define arguments
    iCub.home() # Home by using ARE
    logger.info() << "try to push right with KARMA..."
    if iCub.pushKarma(x, 0, 0.2): # Object will be push to right with y-coordinate as -0.05 from 0.15
        logger.info() << "success"
    else:
        logger.info() << "fail"
    yarp.Time.delay(4.0)

    # push front with pure API of KARMA.Please see KARMA document for details of how to define arguments
    iCub.home() # Home by using ARE
    x[0] = x[0] - 0.15
    logger.info() << "try to push front with KARMA..."
    if iCub.pushKarma(x, -90, 0.2): # Object will be push to front with x-coordinate as -0.45 from -0.25
        logger.info() << "success"
    else:
        logger.info() << "fail"
    yarp.Time.delay(4.0)

    # pull back with pure API of KARMA.Please see KARMA document for details of how to define arguments
    iCub.home() # Home by using ARE
    x[0] = -0.45
    x[1] =  0.1
    x[2] = -0.05
    logger.info() << "try to pull with KARMA..."
    if iCub.drawKarma(x, 0, 0, 0.2): # Object will be pull back 0.2m with x-coordinate from -0.45
        logger.info() << "success"
    else:
        logger.info() << "fail"
    yarp.Time.delay(4.0)

    logger.info() << "shutting down ... "
    iCub.close()
    yarp.Network.fini()
