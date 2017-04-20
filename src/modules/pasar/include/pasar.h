// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
* Copyright (C) 2014 ICUBCLIENT Consortium, European Commission FP7 Project IST-270490
* Authors: Stéphane Lallée, Grégoire Pointeau
* email:   stephane.lallee@gmail.com, gregoire.pointeau@inserm.fr
* website: https://github.com/robotology/icub-client/
* Permission is granted to copy, distribute, and/or modify this program
* under the terms of the GNU General Public License, version 2 or any
* later version published by the Free Software Foundation.
*
* A copy of the license can be found at
* $icub-client/license/gpl.txt
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details
*/

#ifndef _PASAR_MODULE_H_
#define _PASAR_MODULE_H_

#include <utility>
#include "icubclient/clients/icubClient.h"

struct ObjectModel
{
    icubclient::Object o;
    double speed;
    double acceleration;
    int restingSteps;
    double lastTimeSeen;
    bool present;
};


/**
* Module in charge of polling the OPC and updating icubGUI
*/
class PasarModule : public yarp::os::RFModule {
protected:
    //Parameter
    double pTopDownAppearanceBurst;         //!< score of saliency for an appereance
    double pTopDownDisappearanceBurst;      //!< score of saliency for an diappereance
    double pTopDownAccelerationCoef;        //!< score of saliency for an acceleration detected
    double pExponentialDecrease;            //!< Speed of the decrease of the saliency over the time (should be less than 1)
    double pTopDownWaving;                  //!< increase of saliency if waving
    double thresholdMovementAccelAgent;     //!< minimum acceleration to detect an agent
    double thresholdMovementAccelObject;    //!< minimum acceleration to detect an object
    double thresholdWaving;                 //!< minimum waving detected
    double thresholdPointing;               //!< maximum distance between hand and object to detect pointing
    double thresholdSaliency;               //!< if object saliency is below this value, it is set to 0
    double dthresholdAppear;                //!< minimum time an object has to be present before it is detected as appeared
    double dthresholdDisappear;             //!< minimum time an object has to be absent before it is detected as disappeared
    double dBurstOfPointing;                //!< how much to increase saliency if object is pointed at

    double lastTimeWaving;                  //!< last time point when the agent was waving
    double lastTimePointing;                //!< last time point when the agent was pointing

    bool checkWaving;                       //!< whether to detect waving of an agent
    bool checkPointing;                     //!< whether to detect pointing of an agent
    icubclient::ICubClient  *iCub;

    yarp::os::Port handlerPort;             //!< a port to handle messages

    yarp::sig::Vector rightHandt1;          //!< position of right at t1
    yarp::sig::Vector rightHandt2;          //!< position of right at t2
    yarp::sig::Vector leftHandt1;           //!< position of left hand at t1
    yarp::sig::Vector leftHandt2;           //!< position of left hand at t2

    std::pair<bool, bool> presentRightHand; //!< first: whether right hand was present at t1; second: whether right hand is present at t2
    std::pair<bool, bool> presentLeftHand;  //!< first: whether left hand was present at t1; second: whether left hand is present at t2

    std::map<int, ObjectModel>  presentObjectsLastStep;
    std::map<int, std::pair<double, double> > presentLastSpeed;
    std::map<int, std::pair<double, double> > presentCurrentSpeed;
    std::map<int, ObjectModel>  OPCEntities;

    bool isPointing;                        //!< if the human is pointing
    bool isWaving;                          //!< if if the human is waving
    double initTime;

    /**
     * @brief saliencyTopDown Update the salience according to the informations contained in the OPC (acceleration, appareance, disappareance)
     */
    void saliencyTopDown();

    /**
     * @brief saliencyNormalize Normalize salience such that maximum salience = 1
     */
    void saliencyNormalize();

    /**
     * @brief saliencyLeakyIntegration Decrease of the salience through time.
     * Exponential factor pExponentialDecrease < 1
     */
    void saliencyLeakyIntegration();

    /**
     * @brief saliencyPointing Increase the salience of the closest object from the right hand
     * @return True if robot is pointing now, False otherwise
     */
    bool saliencyPointing();

    /**
     * @brief saliencyWaving Increase the saliency of the agent waving
     * @return true is the agent is wavingNow, False otherwise
     */
    bool saliencyWaving();
    void initializeMapTiming();

public:
    bool configure(yarp::os::ResourceFinder &rf); // configure all the module parameters and return true if successful
    bool interruptModule();                       // interrupt, e.g., the ports 
    bool close();                                 // close and shut down the module
    bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);
    double getPeriod();
    bool updateModule();
};

#endif
