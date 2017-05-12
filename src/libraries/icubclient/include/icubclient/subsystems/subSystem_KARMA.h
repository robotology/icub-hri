/*
* Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
* Authors: NGUYEN Dong Hai Phuong
* email:   phuong.nguyen@iit.it
* website: http://wysiwyd.upf.edu/
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

#ifndef SUBSYSTEM_KARMA_H
#define SUBSYSTEM_KARMA_H

#include <string>
#include <algorithm>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

#include "icubclient/subsystems/subSystem.h"
#include "icubclient/subsystems/subSystem_ARE.h"

#define SUBSYSTEM_KARMA       "KARMA"

namespace icubclient {
/**
* \ingroup icubclient_subsystems
*
* SubSystem to deal with the <b>experimental affordance learning</b>
* module (a.k.a. <b>KARMA</b>) for motor control.
*
* For further details, please refer to the KARMA main page:
* http://robotology.github.io/karma/doxygen/doc/html/group__karmaMotor.html
*/
class SubSystem_KARMA : public SubSystem
{
protected:
    SubSystem_ARE* SubARE;
    bool AREconnected;

    std::string robot;
    double tableHeight;
    bool hasTable;

    yarp::os::RpcClient portStop;
    yarp::os::RpcClient portVision;
    yarp::os::RpcClient portFinder;
    yarp::os::RpcClient portCalib;

    //testing Cartesian interface
    yarp::dev::PolyDriver driverL;
    yarp::dev::PolyDriver driverR;
    yarp::dev::PolyDriver driverHL;
    yarp::dev::PolyDriver driverHR;

    yarp::dev::ICartesianControl *iCartCtrlL;
    yarp::dev::ICartesianControl *iCartCtrlR;

    /**
     * @brief Appends a target vector <b>t</b> to the Bottle <b></b>
     */
    void appendTarget(yarp::os::Bottle& b, const yarp::sig::Vector &tCenter);

    /**
     * @brief Appends a double value <b>v</b> to the Bottle <b></b>
     */
    void appendDouble(yarp::os::Bottle& b, const double &v);

    bool returnArmSafely(std::string armType);

    void selectHandCorrectTarget(yarp::os::Bottle& options, const std::string &targetName,
                                 yarp::sig::Vector& target,
                                 const std::string handToUse="");

    /**
     * @brief Sends a command to KARMA's RPC port
     * @param cmd: Command to be sent
     * @return True if successful
     */
    bool sendCmd(yarp::os::Bottle &cmd);

    bool connect();

public:
    /**
    * Default constructor.
    * @param masterName stem-name used to open up ports.
    * @param robot name of the robot (`icub` or `icubSim`)
    */
    SubSystem_KARMA(const std::string &masterName, const std::string &robot);

    void Close();

    /**
     * @brief Applies safety margins, i.e. the *in* vector should be at least table height and at least 0.1m in front of the iCub
     * @param in: "Unsafe" vector
     * @return "safe" vector
     */
    yarp::sig::Vector applySafetyMargins(const yarp::sig::Vector& in);

    /**
     * @brief chooseArm (toolAttach in KARMA): wrapper for tool-attach of KARMA, can be used to choose the arm for actions with KARMA
     * @param armType: string value of "left" or "right" arm
     */
    bool chooseArm(const std::string &armType);

    /**
     * @brief chooseArmAuto (toolRemove in KARMA): wrapper for tool-remove of Karma, use to clear the arm choise
     */
    void chooseArmAuto();

    /**
     * @brief pushAside (KARMA): push an object to a certain location along y-axis of robot RoF
     * @param objName: name of object (can be empty, in this case no correction is applied by iolReachingCalibration)
     * @param objCenter: coordinate of object
     * @param targetPosY: y coordinate of location to push object to
     * @param theta: angle to define pushing left (0) or right (180)
     * @param armType: "left" or "right" arm to conduct action, otherwise arm will be chosen by KARMA
     * @param options to be passed to KARMA
     * @return true in case of success release, false otherwise
     */
    bool pushAside(const std::string &objName,
                   const yarp::sig::Vector &objCenter, const double &targetPosY,
                   const double &theta,
                   const std::string &armType = "selectable",
                   const yarp::os::Bottle &options = yarp::os::Bottle());

    /**
     * @brief pushFront (KARMA): push an object to a certain location along x-axis of robot RoF
     * @param objName: name of object (can be empty, in this case no correction is applied by iolReachingCalibration)
     * @param objCenter: coordinate of object
     * @param targetPosXFront: x coordinate of location to push object to
     * @param armType: "left" or "right" arm to conduct action, otherwise arm will be chosen by KARMA
     * @param options to be passed to KARMA
     * @return true in case of success release, false otherwise
     */
    bool pushFront(const std::string &objName,
                   const yarp::sig::Vector &objCenter, const double &targetPosXFront,
                   const std::string &armType = "selectable",
                   const yarp::os::Bottle &options = yarp::os::Bottle());

    /**
     * @brief push (KARMA): push to certain position, along a defined direction
     * @param targetCenter: position to push to.
     * @param theta: angle between the y-axis (in robot FoR) and starting position of push action, defines the direction of push action
     * @param radius: radius of the circle with center at @see targetCenter
     * @param options to be passed to KARMA
     * @return true in case of success release, false otherwise
     */
    bool push(const yarp::sig::Vector &targetCenter, const double theta, const double radius,
              const yarp::os::Bottle &options = yarp::os::Bottle());

    /**
     * @brief pullBack (KARMA): pull an object to a certain location along x-axis of robot RoF
     * @param objName: name of object to pull (can be empty, in this case no correction is applied by iolReachingCalibration)
     * @param objCenter: coordinate of object
     * @param targetPosXBack: x coordinate of location to pull object to
     * @param armType: "left" or "right" arm to conduct action, otherwise arm will be chosen by KARMA
     * @param options to be passed to KARMA
     * @return true in case of success release, false otherwise
     */
    bool pullBack(const std::string &objName,
                  const yarp::sig::Vector &objCenter, const double &targetPosXBack,
                  const std::string &armType = "selectable",
                  const yarp::os::Bottle &options = yarp::os::Bottle());
    /**
     * @brief draw (KARMA): draw action, along the positive direction of the x-axis (in robot FoR)
     * @param targetCenter: center of a circle
     * @param theta: angle between the y-axis (in robot FoR) and starting position of draw action.
     * @param radius: radius of the circle with center at @see targetCenter
     * @param dist: moving distance of draw action
     * @param options to be passed to KARMA
     * @return true in case of success release, false otherwise
     */
    bool draw(const yarp::sig::Vector &targetCenter, const double theta,
              const double radius, const double dist,
              const yarp::os::Bottle &options = yarp::os::Bottle());

    /**
     * @brief vdraw (KARMA): virtual draw action, along the positive direction of the x-axis (in robot FoR)
     * @param targetCenter: center of a circle
     * @param theta: angle between the y-axis (in robot FoR) and starting position of draw action.
     * @param radius: radius of the circle with center at @see targetCenter
     * @param dist: moving distance of draw action
     * @param options to be passed to KARMA
     * @param targetName: name of object to push (can be empty, in this case no correction is applied by iolReachingCalibration)
     * @return true in case of success release, false otherwise
     */
    bool vdraw(const std::string &objName,
               const yarp::sig::Vector &targetCenter, const double theta,
               const double radius, const double dist,
               const yarp::os::Bottle &options = yarp::os::Bottle());

    bool openCartesianClient();

    /**
    * Destructor.
    */
    ~SubSystem_KARMA();
};
}

#endif // SUBSYSTEM_KARMA_H
