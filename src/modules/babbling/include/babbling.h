/* 
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Martina Zambelli
 * email:   m.zambelli13@imperial.ac.uk
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * icub-client/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#ifndef _BABBLING_H_
#define _BABBLING_H_

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>

/**
 * @ingroup babbling
 */
class Babbling : public yarp::os::RFModule {
protected:
    yarp::os::RpcServer handlerPort;

    yarp::dev::IPositionControl* posLeftArm;
    yarp::dev::IVelocityControl* velLeftArm;
    yarp::dev::ITorqueControl *itrqLeftArm;
    yarp::dev::IEncoders* encsLeftArm;
    yarp::dev::IControlMode *ictrlLeftArm;
    yarp::dev::IControlLimits *ictrlLimLeftArm;

    yarp::dev::IPositionControl* posRightArm;
    yarp::dev::IVelocityControl* velRightArm;
    yarp::dev::ITorqueControl *itrqRightArm;
    yarp::dev::IEncoders* encsRightArm;
    yarp::dev::IControlMode *ictrlRightArm;
    yarp::dev::IControlLimits *ictrlLimRightArm;

    yarp::dev::IGazeControl* igaze;

    yarp::dev::PolyDriver leftArmDev;
    yarp::dev::PolyDriver rightArmDev;
    yarp::dev::PolyDriver headDev;

    yarp::sig::VectorOf<double> encodersLeftArm, encodersRightArm;

    std::string part; //!< Should be "left_arm" or "right_arm"
    std::string robot; //!< Should be "icub" or "icubSim"
    std::string part_babbling; //!< Whether to do babbling with the whole "arm" or "hand" (should be either one or the other)
    int single_joint; //!< Which joint to do babbling with in case it is a single joint

    double freq; //!< Frequency for sin wave
    double amp; //!< Amplitude for sin wave
    double duration; //!< Duration for the babbling
    
    double start_command_arm[16]; //!< Start command for the 16 arm joints
    yarp::sig::VectorOf<double> start_command_head; //!< Target for head in start position

public:
    bool configure(yarp::os::ResourceFinder &rf);
    bool interruptModule();
    bool close();
    bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);
    double getPeriod();
    bool updateModule();

protected:
    /**
     * @brief Initializes all the device controllers
     * @return True if successful
     */
    bool init_iCub();
    bool init_left_arm(); //!< Create PolyDriver for left arm
    bool init_right_arm(); //!< Create PolyDriver for left arm

    /**
     * @brief Sets the joints to be in velocity control, then goes in a loop to call `babblingCommands` for `duration` seconds
     * @return True if successful, false if not
     */
    bool doBabbling();


    /**
     * @brief Issues the actual velocity commands for time `t`.
     * @param t - time to be used for the sin wave
     * @param j_idx - if -1, do whole arm / whole hand babbling, otherwise babble single joint
     */
    void babblingCommands(double &t, int j_idx);

    /**
     * @brief Use iGazeController to move head in start position
     * @return true if successful
     */
    bool moveHeadToStartPos();

    /**
     * @brief Move head + arm in start position
     * @return true if successful
     */
    bool gotoStartPos();
};

#endif // _BABBLING_H_

