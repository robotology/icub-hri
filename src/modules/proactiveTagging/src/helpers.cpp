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

#include "proactiveTagging.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace icubclient;

void proactiveTagging::subPopulateObjects(Bottle* objectList, bool addOrRetrieve) {
    if (objectList) {
        for (int d = 0; d < objectList->size(); d++) {
            std::string name = objectList->get(d).asString().c_str();
            icubclient::Object* o;
            if(addOrRetrieve) {
                o = iCub->opc->addOrRetrieveEntity<Object>(name);
            } else {
                o = iCub->opc->addEntity<Object>(name);
            }
            yInfo() << " [configureOPC] object " << o->name() << "added" ;
            o->m_present = 0.0;
            iCub->opc->commit(o);
        }
    }
}

bool proactiveTagging::setPasarPointing(bool on) {
    Bottle bToPasar, bFromPasar;
    bToPasar.addString("pointing");
    if(on) {
        bToPasar.addString("on");
    } else {
        bToPasar.addString("off");
    }
    if (!Network::connect(portToPasar.getName().c_str(), "/pasar/rpc")) {
        yError() << "Could not connect to pasar";
        iCub->say("Could not connect to pasar");
        return false;
    } else {
        portToPasar.write(bToPasar, bFromPasar);
        if(bFromPasar.get(0).asString()!="ack") {
            yError() << "Pasar did not change pointing to on";
            iCub->say("Pasar did not change pointing to on");
            return false;
        } else {
            return true;
        }
    }
}

std::string proactiveTagging::getBestEntity(std::string sTypeTarget) {
    bool bFound = false;
    std::string sNameBestEntity = "none", sNameSecondBest = "none";

    double start = yarp::os::Time::now();
    // start detecting unknown objects
    while (!bFound && start + 8.0 > yarp::os::Time::now()) {
        iCub->opc->checkout();
        std::list<std::shared_ptr<Entity>> lEntities = iCub->opc->EntitiesCacheCopy();

        double highestSaliency = 0.0;
        double secondSaliency = 0.0;

        for (auto& entity : lEntities) {
            if (entity->name().find("unknown")==0) {
                if (sTypeTarget == ICUBCLIENT_OPC_ENTITY_OBJECT && (entity->entity_type() == ICUBCLIENT_OPC_ENTITY_OBJECT)) {
                    Object* temp = dynamic_cast<Object*>(entity.get());
                    if(!temp) {
                        yError() << "Could not cast " << entity->name() << " to an object";
                        iCub->say("Could not cast " + entity->name() + " to an object");
                    } else {
                        if (temp->m_saliency > highestSaliency) {
                            if (secondSaliency != 0.0) {
                                secondSaliency = highestSaliency;
                            }
                            highestSaliency = temp->m_saliency;
                            sNameBestEntity = temp->name();
                        } else {
                            if (temp->m_saliency > secondSaliency) {
                                secondSaliency = temp->m_saliency;
                                sNameSecondBest = temp->name();
                            }
                        }
                    }
                }
            }
        }

        yDebug() << sNameBestEntity << " has highest saliency: " << highestSaliency;
        yDebug() << sNameSecondBest << " has second highest saliency: " << secondSaliency;

        bFound = false;
        if (highestSaliency > thresholdSalienceDetection) {
            //the object with highest salience is salient enough
            if (secondSaliency != 0.0) {
                // there are other salient objects
                if ((highestSaliency / secondSaliency) > thresholdDistinguishObjectsRatio) {
                    yDebug() << "Two objects are salient, but one is much more";
                    //but it is enough difference
                    bFound = true;
                } else {
                    yDebug() << "Two objects are similarly salient";
                }
            } else {
                yDebug() << "Only one object is salient, take this one";
                //other object are not salient
                bFound = true;
            }
        }
        if (sNameBestEntity == "none") {
            yDebug() << "No object is salient";
            bFound = false;
        }
    }

    return sNameBestEntity;
}

Bottle proactiveTagging::recogName(std::string entityType) {
    Bottle bOutput;

    Bottle bRecognized, //received from speech recog with transfer information (1/0 (bAnswer))
           bAnswer;     //response from speech recog without transfer information, including raw sentence

    yDebug() << "Going to load grammar.";
    string grammar, expectedresponse="error", semanticfield="error";
    if (entityType == ICUBCLIENT_OPC_ENTITY_AGENT) {
        grammar = GrammarAskNameAgent;
        expectedresponse = "SENTENCEAGENT";
        semanticfield = "agent";
    } else if (entityType == ICUBCLIENT_OPC_ENTITY_OBJECT) {
        grammar = GrammarAskNameObject;
        expectedresponse = "SENTENCEOBJECT";
        semanticfield = "object";
    } else if (entityType == ICUBCLIENT_OPC_ENTITY_BODYPART) {
        grammar = GrammarAskNameBodypart;
        expectedresponse = "SENTENCEBODYPART";
        semanticfield = "fingerName";
    } else {
        yError() << " error in proactiveTagging::recogName | for " << entityType << " | Entity Type not managed";
        bOutput.addString("error");
        bOutput.addString("Entity Type not managed");
        iCub->say("I do not know what you want from me. Can you please ask me something else?");
        return bOutput;
    }

    // Load the Speech Recognition with grammar according to entityType
    // bAnswer is the result of the recognition system (first element is the raw sentence, second is the list of semantic element)
    bool recognizedCorrectGrammar=false;
    while(!recognizedCorrectGrammar) {
        bRecognized = iCub->getRecogClient()->recogFromGrammarLoop(grammarToString(grammar), 20);
        bAnswer = *bRecognized.get(1).asList();
        if(bAnswer.get(1).asList()->get(0).toString() != expectedresponse) {
            iCub->say("I did not understand you. Can you please repeat?");
            yError() << "Wrong sentence type returned (not " << expectedresponse << ")";
        } else {
            recognizedCorrectGrammar = true;
        }
    }

    yDebug() << "Response from recogClient: " << bRecognized.toString();

    if (bRecognized.get(0).asInt() == 0)
    {
        yError() << " error in proactiveTagging::askName | for " << entityType << " | Error in speechRecog";
        bOutput.addString("error");
        bOutput.addString("error in speechRecog");
        return bOutput;
    }

    iCub->say("I've understood " + bAnswer.get(0).asString());

    Bottle bSemantic = *bAnswer.get(1).asList(); // semantic information of the content of the recognition
    string sName = bSemantic.check(semanticfield, Value("error")).asString();
    bOutput.addString(sName);
    return bOutput;
}

void proactiveTagging::subPopulateBodyparts(Bottle* bodyPartList, Bottle* bodyPartJointList, bool addOrRetrieve) {
    std::list<std::shared_ptr<Entity>> currentEntitiesList = iCub->opc->EntitiesCacheCopy();

    if (bodyPartList) {
        for (int d = 0; d < bodyPartList->size(); d++) {
            bool foundSame = false;
            for(auto& e : currentEntitiesList) {
                if(Bodypart* bp = dynamic_cast<Bodypart*>(e.get())) {
                    if(bp->m_joint_number == bodyPartJointList->get(d).asInt()) {
                        yWarning() << "Joint" << bp->m_joint_number << "already existing";
                        foundSame = true;
                        break;
                    }
                }
            }
            if(foundSame) {
                continue;
            }
            std::string name = bodyPartList->get(d).asString().c_str();
            icubclient::Bodypart* o;
            if(addOrRetrieve) {
                o = iCub->opc->addOrRetrieveEntity<Bodypart>(name);
            } else {
                o = iCub->opc->addEntity<Bodypart>(name);
            }
            yInfo() << " [configureOPC] Bodypart " << o->name() << "added";
            o->m_present = 0.0;
            //apply the joint number if available. protect for the loop because using d from bodyPartList. should be same number of element between bodyPartList and bodyPartJointList
            if(d < bodyPartJointList->size()) {
                o->m_joint_number = bodyPartJointList->get(d).asInt();
                yInfo() << " [configureOPC] Bodypart " << o->name() << " has now a joint " << o->m_joint_number ;
            }
            iCub->opc->commit(o);
        }
    }
}

void proactiveTagging::configureOPC(yarp::os::ResourceFinder &rf) {
    //Populate the OPC if required
    yDebug() << "Populating OPC...";

    //1. Populate AddOrRetrieve part
    Bottle grpOPC_AOR = rf.findGroup("OPC_AddOrRetrieve");
    bool shouldPopulate_AOR = grpOPC_AOR.find("populateOPC").asInt() == 1;
    if (shouldPopulate_AOR) {
        Bottle *objectList = grpOPC_AOR.find("objectName").asList();
        subPopulateObjects(objectList, true);

        Bottle *bodyPartList = grpOPC_AOR.find("bodypartName").asList();
        Bottle *bodyPartJointList = grpOPC_AOR.find("bodypartJoint").asList();
        subPopulateBodyparts(bodyPartList, bodyPartJointList, true);
    }

    //2. Populate Add part (allows several object with same base name, e.g. object, object_1, object_2, ..., object_n)
    Bottle grpOPC_Add = rf.findGroup("OPC_Add");
    bool shouldPopulate_Add = grpOPC_Add.find("populateOPC").asInt() == 1;
    if (shouldPopulate_Add) {
        Bottle *objectList = grpOPC_Add.find("objectName").asList();
        subPopulateObjects(objectList, false);

        Bottle *bodyPartList = grpOPC_Add.find("bodypartName").asList();
        Bottle *bodyPartJointList = grpOPC_Add.find("bodypartJoint").asList();
        subPopulateBodyparts(bodyPartList, bodyPartJointList, false);
    }

    yDebug() << "configureOPC done";
}

std::string proactiveTagging::getBodyPartNameForSpeech(const std::string bodypart) {
    std::string out = bodypart;
    if(bodypart == "index" || bodypart == "middle" || bodypart == "ring" || bodypart == "little") {
        out = out + " finger";
    }
    return out;
}
