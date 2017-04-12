/*
 * Copyright (C) 2015 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Grégoire Pointeau, Tobias Fischer, Maxime Petit
 * email:   greg.pointeau@gmail.com, t.fischer@imperial.ac.uk, m.petit@imperial.ac.uk
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

#include "icubclient/clients/icubClient.h"
#include "icubclient/subsystems/subSystem_recog.h"
#include "icubclient/subsystems/subSystem_speech.h"

/**
 * \ingroup proactiveTagging
 */
class proactiveTagging : public yarp::os::RFModule {
private:

    icubclient::ICubClient  *iCub; //!< icubclient

    double      period;

    std::string SAMRpc; //!< Name of SAM RPC port
    std::string touchDetectorRpc; //!< Name of touchDetector RPC port

    yarp::os::Port   rpcPort; //!< Response port
    yarp::os::Port   portToSAM; //!< Port used to communicate with SAM
    yarp::os::Port   portToPasar; //!< Port used to communicate with pasar
    yarp::os::BufferedPort<yarp::os::Bottle>   portFromTouchDetector;  //!< Port used to communicate with touch detector

    std::string      GrammarAskNameAgent; //!< Name of grammar file when asking for name of agent
    std::string      GrammarAskNameObject; //!< Name of grammar file when asking for name of object
    std::string      GrammarAskNameBodypart; //!< Name of grammar file when asking for name of bodypart

    std::string      babblingArm; //!< side of the babbling arm : left or right

    double  thresholdDistinguishObjectsRatio; //!< ratio of saliency needed to detect if 1 object is more salient that the other
    double  thresholdSalienceDetection; //!< value of saliency needed to detect if 1 object is more salient that the other

    //Configure
    /**
     * @brief Read config file and fill OPC accordingly
     * @param rf - resourceFinder instance
     */
    void configureOPC(yarp::os::ResourceFinder &rf);
    void subPopulateObjects(yarp::os::Bottle* objectList, bool addOrRetrieve);
    void subPopulateBodyparts(yarp::os::Bottle* bodyPartList, yarp::os::Bottle* bodyPartJointList, bool addOrRetrieve);

    //objectTagging
    /**
    * @brief Recognize the name of an unknown entity (communicate with speech recognizer)
    * @return Bottle with either name of the object or (error errorDescription)
    */
    yarp::os::Bottle   recogName(std::string entityType);

    /**
     * @brief Return a "nice" version of a body part name, eg given "index" as input, it returns "index finger"
     * @param bodypart - "Plain" version of a body part name, eg "index"
     * @return "Nice" version of a body part name for speech synthesis, eg "index finger"
     */
    std::string getBodyPartNameForSpeech(const std::string bodypart);

    /**
     * @brief Ask pasar to increase saliency when a human is pointing to an object
     * @param on - whether pasar should increase the saliency
     * @return True if communication to pasar was successful, false otherwise
     */
    bool setPasarPointing(bool on);

public:
    /**
    * @brief Explore an unknown entity by asking for the name (response via speech recognition)
    * @param bInput: exploreUnknownEntity entityType entityName (eg: exploreUnknownEntity agent unknown_25)
    * @return Bottle with (success entityType) or (nack errorMessage)
    */
    yarp::os::Bottle  exploreUnknownEntity(const yarp::os::Bottle &bInput);

    /**
    * Search for the entity corresponding to a certain name in all the unknown entities
    * @param bInput: searchingEntity entityType entityName [verbose] (eg: searchingEntity object octopus)
    * @return Bottle consisting of two elements. First element is {error; warning; success} Second element is: information about the action
    */
    yarp::os::Bottle  searchingEntity(const yarp::os::Bottle &bInput);

    /**
     * @brief Loop through all objects in the OPC, and check their saliency. Return the name of the object with the highest saliency
     * @param sTypeTarget - type of the target. For now, only "object" is supported
     * @return Name of the most salient object
     */
    std::string getBestEntity(std::string sTypeTarget);


    /**
     * @brief Send request to SAM to use its face recognition to recognise partner
     * @param sNameTarget: name of the entity to detect, typically "partner"
     * @param currentEntityType: type of the entity, typically "agent"
     * @return Bottle. In case of success: (success entityType) where entityType = agent, in case of failure: (nack)
     */
    yarp::os::Bottle getNameFromSAM(std::string sNameTarget, std::string currentEntityType);

    /**
    * @brief Explore an unknown tactile entity (e.g. fingertips), when knowing the name
    * @param bInput: Bottle with (exploreTactileEntityWithName entityType entityName) (eg: exploreTactileEntityWithName bodypart index) - entityType must be "bodypart"!
    * @return Bottle with the result (ack skin_patch_number) or (nack error_message)
    */
    yarp::os::Bottle exploreTactileEntityWithName(yarp::os::Bottle bInput);

    bool configure(yarp::os::ResourceFinder &rf);

    bool interruptModule();

    bool close();

    double getPeriod() {
        return period;
    }

    bool updateModule();

    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);
};
