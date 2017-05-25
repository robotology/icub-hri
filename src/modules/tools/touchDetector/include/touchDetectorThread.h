#ifndef RPCLISTENERTHREADH
#define RPCLISTENERTHREADH

/* 
 * Copyright (C) 2015 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Bertand HIGY
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

#include <string>
#include <vector>
#include <exception>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/RateThread.h>

/**
 * @ingroup touchDetector
 */
class TouchDetectorThread : public yarp::os::RateThread
{
    public:
        TouchDetectorThread(yarp::os::BufferedPort<yarp::os::Bottle> *torsoPort, yarp::os::BufferedPort<yarp::os::Bottle> *leftArmPort, yarp::os::BufferedPort<yarp::os::Bottle> *rightArmPort, yarp::os::BufferedPort<yarp::os::Bottle> *leftForearmPort, yarp::os::BufferedPort<yarp::os::Bottle> *rightForearmPort, yarp::os::BufferedPort<yarp::os::Bottle> *leftHandPort, yarp::os::BufferedPort<yarp::os::Bottle> *rightHandPort, yarp::os::BufferedPort<yarp::os::Bottle> *touchPort, yarp::os::BufferedPort<yarp::os::Bottle> *touchPortCleaned, int period, std::string clustersConfFilepath, double threshold, int taxelThreshold);
        void run(); 
        bool threadInit();
       
    protected:
        static const int noCluster;
        static const int nbBodyParts;
        static const char* bodyParts[7];
        static const int nbTaxels[7];
        
        int nbClusters;
        double threshold;
        int taxelThreshold;                                             //!< integer value of touched threshold
        std::string clustersConfFilepath;                               //!< string value of the path to configure file of clusters
        std::vector<int> taxels2Clusters[7];                            //!< standard vector of integer containing the taxel IDs in form of cluster ID
        
        /* ports */
        yarp::os::BufferedPort<yarp::os::Bottle> *torsoPort;            //!< Yarp BufferedPort of bottle for input torso activations
        yarp::os::BufferedPort<yarp::os::Bottle> *leftArmPort;          //!< Yarp BufferedPort of bottle for input left arm activations
        yarp::os::BufferedPort<yarp::os::Bottle> *rightArmPort;         //!< Yarp BufferedPort of bottle for input right arm activations
        yarp::os::BufferedPort<yarp::os::Bottle> *leftForearmPort;      //!< Yarp BufferedPort of bottle for input left forearm activations
        yarp::os::BufferedPort<yarp::os::Bottle> *rightForearmPort;     //!< Yarp BufferedPort of bottle for input right forearm activations
        yarp::os::BufferedPort<yarp::os::Bottle> *leftHandPort;         //!< Yarp BufferedPort of bottle for input left hand activations
        yarp::os::BufferedPort<yarp::os::Bottle> *rightHandPort;        //!< Yarp BufferedPort of bottle for input right hand activations
        yarp::os::BufferedPort<yarp::os::Bottle> *touchPort;            //!< Yarp BufferedPort of bottle for output touched activations
        yarp::os::BufferedPort<yarp::os::Bottle> *touchPortCleaned;     //!< Yarp BufferedPort of bottle for output touched taxel
        
        /**
         * @brief readTaxelsMapping Read taxels map from configure file, which relates to nbBodyParts and bodyParts
         * @param filename Name of configure file of taxels
         * @return True for all case
         */
        bool readTaxelsMapping(std::string filename);

        /**
         * @brief getBodyPartId Obtain the ID of body parts, which are "torso", "left_arm", "right_arm", "left_forearm", "right_forearm", "left_hand", "right_hand"
         * @param bodyPartName String value of the name of body part to get ID
         * @return Integer value of ID
         */
        int getBodyPartId(std::string bodyPartName);

        /**
         * @brief updateMapping Change all taxels of a body part to clusterID
         * @param bodyPart An integer value of body part ID
         * @param firstTaxel An integer value of the 1st taxel of the body part
         * @param lastTaxel An integer value of the last taxel of the body part
         * @param cluster An integer value of the cluster ID
         * @see taxels2Clusters
         */
        void updateMapping(int bodyPart, int firstTaxel, int lastTaxel, int cluster);

        /**
         * @brief countActivations Counts number of activations to a body part through a buffered port of that part
         * @param bodyPart An integer value of body part ID to count
         * @param data YARP bottle contains data from the port of the body part
         * @param activations An integer value for number of activations
         */
        void countActivations(int bodyPart, yarp::os::Bottle *data, std::vector<int> &activations);

        /**
         * @brief processPort Process port of body part to receive activations to that body part
         * @param portNum An integer value of body part ID to count
         * @param activations A vector of integer values for number of activations
         * @param port The port from which to process data from
         * @see countActivations();
         */
        void processPort(int portNum, yarp::os::BufferedPort<yarp::os::Bottle> *port, std::vector<int> &activations);
};

class ParsingException: public std::exception
{
    public:
        int line;
        
        ParsingException();
        virtual const char* what() const throw();
};

class BadFormatException: public std::exception
{
public:
    std::string expectedType;
    std::string portName;

    BadFormatException();
    virtual const char* what() const throw();
};

#endif
