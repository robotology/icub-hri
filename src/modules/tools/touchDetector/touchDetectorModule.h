#ifndef RPCLISTENERMODULEH
#define RPCLISTENERMODULEH

/* 
 * Copyright (C) 2015 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Bertand Higy
 * email:  bertrand.higy@iit.it
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/RFModule.h>

#include "touchDetectorThread.h"

class TouchDetectorModule: public yarp::os::RFModule
{
    public:
        bool configure(yarp::os::ResourceFinder &rf); // configure all the module parameters and return true if successful
        bool interruptModule();                       // interrupt, e.g., the ports 
        bool close();                                 // close and shut down the module
        //bool respond();
        bool updateModule();
       
    protected:
        /* module parameters */
        int period;
        double threshold;
        int taxelThreshold;
        std::string moduleName;
        std::string torsoPortName;                                      //!< name of torso port
        std::string leftArmPortName;                                    //!< name of left arm port
        std::string rightArmPortName;                                   //!< name of right arm port
        std::string leftForearmPortName;                                //!< name of left forearm port
        std::string rightForearmPortName;                               //!< name of right forearm port
        std::string leftHandPortName;                                   //!< name of left hand port
        std::string rightHandPortName;                                  //!< name of right hand port
        std::string touchPortName;                                      //!< name of touchport
        std::string touchPortCleanName;                                 //!< name of touchportClean
        std::string clustersConfFilepath;                               //!< string for path to cluster configure file

        /* class variables */
        yarp::os::BufferedPort<yarp::os::Bottle> torsoPort;             //!< Yarp BufferedPort of bottle for input torso activations
        yarp::os::BufferedPort<yarp::os::Bottle> leftArmPort;           //!< Yarp BufferedPort of bottle for input left arm activations
        yarp::os::BufferedPort<yarp::os::Bottle> rightArmPort;          //!< Yarp BufferedPort of bottle for input right arm activations
        yarp::os::BufferedPort<yarp::os::Bottle> leftForearmPort;       //!< Yarp BufferedPort of bottle for input left forearm activations
        yarp::os::BufferedPort<yarp::os::Bottle> rightForearmPort;      //!< Yarp BufferedPort of bottle for input right forearm activations
        yarp::os::BufferedPort<yarp::os::Bottle> leftHandPort;          //!< Yarp BufferedPort of bottle for input left hand activations
        yarp::os::BufferedPort<yarp::os::Bottle> rightHandPort;         //!< Yarp BufferedPort of bottle for input right hand activations
        yarp::os::BufferedPort<yarp::os::Bottle> touchPort;             //!< Yarp BufferedPort of bottle for output touched activations
        yarp::os::BufferedPort<yarp::os::Bottle> touchPortCleaned;      //!< Yarp BufferedPort of bottle for output touched taxel
        TouchDetectorThread *thread;
        
        void initializeParameters(yarp::os::ResourceFinder &rf);
        bool openPorts();
};

#endif
