/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Stéphane Lallée
 * email:   stephane.lallee@gmail.com
 * website: https://github.com/robotology/icub-client/
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

#include <yarp/os/LogStream.h>

#include "icubhri/clients/icubClient.h"
#include "icubhri/subsystems/subSystem_agentDetector.h"
#include "icubhri/subsystems/subSystem_ARE.h"
#include "icubhri/subsystems/subSystem_babbling.h"
#include "icubhri/subsystems/subSystem_iol2opc.h"
#include "icubhri/subsystems/subSystem_speech.h"
#include "icubhri/subsystems/subSystem_recog.h"
#include "icubhri/subsystems/subSystem_KARMA.h"
#include "icubhri/subsystems/subSystem_SAM.h"
#include "icubhri/subsystems/subSystem_emotion.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace icubhri;

ICubClient::ICubClient(const std::string &moduleName, const std::string &context, const std::string &clientConfigFile, bool isRFVerbose)
{
    yarp::os::ResourceFinder rfClient;
    rfClient.setVerbose(isRFVerbose);
    rfClient.setDefaultContext(context.c_str());
    rfClient.setDefaultConfigFile(clientConfigFile.c_str());
    rfClient.configure(0, nullptr);

    if (rfClient.check("robot"))
    {
        robot = rfClient.find("robot").asString();
        yInfo("Robot name set to %s", robot.c_str());
    }
    else
    {
        robot = "icub";
        yInfo("Robot name set to default, i.e. %s", robot.c_str());
    }

    //Reaching range
    Bottle defaultRangeMin; defaultRangeMin.fromString("-0.5 -0.3 -0.15");
    Bottle defaultRangeMax; defaultRangeMax.fromString("-0.1 0.3 0.5");
    Bottle *rangeMin = rfClient.find("reachingRangeMin").asList();
    Bottle *rangeMax = rfClient.find("reachingRangeMax").asList();
    if (rangeMin == nullptr) rangeMin = new Bottle(defaultRangeMin);
    if (rangeMax == nullptr) rangeMax = new Bottle(defaultRangeMax);
    xRangeMin = defaultRangeMin.get(0).asDouble(); xRangeMax = defaultRangeMax.get(0).asDouble();
    yRangeMin = defaultRangeMin.get(1).asDouble(); yRangeMax = defaultRangeMax.get(1).asDouble();
    zRangeMin = defaultRangeMin.get(2).asDouble(); zRangeMax = defaultRangeMax.get(2).asDouble();

    icubAgent = nullptr;

    //OPC
    string fullName = moduleName + "/icubClient";
    opc = new OPCClient(fullName);
    opc->isVerbose = false;

    //Susbsystems
    if (Bottle* bSubsystems = rfClient.find("subsystems").asList())
    {
        for (int s = 0; s < bSubsystems->size(); s++)
        {
            std::string currentSS = bSubsystems->get(s).asString();
            yInfo() << "Trying to open subsystem : " << currentSS;
            if (currentSS == SUBSYSTEM_SPEECH)
                subSystems[SUBSYSTEM_SPEECH] = new SubSystem_Speech(fullName);
            else if (currentSS == SUBSYSTEM_ARE)
                subSystems[SUBSYSTEM_ARE] = new SubSystem_ARE(fullName);
            else if (currentSS == SUBSYSTEM_RECOG)
                subSystems[SUBSYSTEM_RECOG] = new SubSystem_Recog(fullName);
            else if (currentSS == SUBSYSTEM_IOL2OPC)
                subSystems[SUBSYSTEM_IOL2OPC] = new SubSystem_IOL2OPC(fullName);
            else if (currentSS == SUBSYSTEM_AGENTDETECTOR)
                subSystems[SUBSYSTEM_AGENTDETECTOR] = new SubSystem_agentDetector(fullName);
            else if (currentSS == SUBSYSTEM_BABBLING)
                subSystems[SUBSYSTEM_BABBLING] = new SubSystem_babbling(fullName);
            else if (currentSS == SUBSYSTEM_KARMA)
                subSystems[SUBSYSTEM_KARMA] = new SubSystem_KARMA(fullName, robot);
            else if (currentSS == SUBSYSTEM_SAM)
                subSystems[SUBSYSTEM_SAM] = new SubSystem_SAM(fullName);
            else if (currentSS == SUBSYSTEM_EMOTION)
                subSystems[SUBSYSTEM_EMOTION] = new SubSystem_emotion(fullName);
            else
                yError() << "Unknown subsystem!";
        }
    }

    closed = false;
}

bool ICubClient::connectOPC(const string &opcName)
{
    bool isConnected = opc->connect(opcName);
    if (isConnected)
        updateAgent();
    yInfo() << "Connection to OPC: " << (isConnected ? "successful" : "failed");
    return isConnected;
}


bool ICubClient::connectSubSystems()
{
    bool isConnected = true;
    for (map<string, SubSystem*>::iterator sIt = subSystems.begin(); sIt != subSystems.end(); sIt++)
    {
        yInfo() << "Connection to " << sIt->first << ": ";
        bool result = sIt->second->Connect();
        yInfo() << (result ? "successful" : "failed");
        isConnected &= result;
    }

    return isConnected;
}


bool ICubClient::connect(const string &opcName)
{
    bool isConnected = connectOPC(opcName);
    isConnected &= connectSubSystems();
    return isConnected;
}


void ICubClient::close()
{
    if (closed)
        return;

    yInfo() << "Terminating subsystems:";
    for (map<string, SubSystem*>::iterator sIt = subSystems.begin(); sIt != subSystems.end(); sIt++)
    {
        yInfo() << "\t" << sIt->first;
        sIt->second->Close();
        if(sIt->second) {
            delete sIt->second;
        }
    }

    opc->interrupt();
    opc->close();

    closed = true;
}

ICubClient::~ICubClient() {
    close();
    if(opc) {
        delete opc;
    }
}


void ICubClient::updateAgent()
{
    if (opc->isConnected())
    {
        if (this->icubAgent == nullptr)
        {
            icubAgent = opc->addOrRetrieveEntity<Agent>("icub");
        }
        else
            opc->update(this->icubAgent);
    }
    opc->Entities(ICUBHRI_OPC_ENTITY_TAG, "==", ICUBHRI_OPC_ENTITY_ACTION);
}

bool ICubClient::changeName(Entity *e, const std::string &newName) {
    bool allOkay = true;
    if (e->entity_type() == ICUBHRI_OPC_ENTITY_AGENT) {
        if (subSystems.find("agentDetector") == subSystems.end()) {
            say("Could not change name of default partner of agentDetector");
            yWarning() << "Could not change name of default partner of agentDetector";
            opc->changeName(e, newName);
            opc->commit(e);

            allOkay = false;
        }
        else {
            SubSystem_agentDetector* sub_agent = dynamic_cast<SubSystem_agentDetector*>(subSystems["agentDetector"]);
            if(!sub_agent) {
                yError() << "Could not cast to SubSystem_agentDetector!";
                return false;
            }

            sub_agent->pause();

            opc->changeName(e, newName);
            opc->commit(e);

            if (!sub_agent->changeDefaultName(newName)) {
                say("could not change default name of partner");
                yError() << "[SubSystem_agentDetector] Could not change default name of partner";
                allOkay = false;
            }

            sub_agent->resume();
        }
    }
    else if (e->entity_type() == ICUBHRI_OPC_ENTITY_OBJECT) {
        if (subSystems.find("iol2opc") == subSystems.end()) {
            say("iol2opc not reachable, did not change object name");
            yWarning() << "iol2opc not reachable, did not change object name";
            opc->changeName(e, newName);
            opc->commit(e);

            allOkay = false;
        }
        else {
            SubSystem_IOL2OPC* sub_iol2opc = dynamic_cast<SubSystem_IOL2OPC*>(subSystems["iol2opc"]);
            string oldName = e->name();
            if (!sub_iol2opc || !sub_iol2opc->changeName(oldName, newName)) {
                yError() << "iol2opc did not change name successfuly";
                say("iol2opc did not change name successfuly");
                allOkay = false;
            }
        }
    }
    else {
        if (!opc->changeName(e, newName)) {
            yError() << "Could not change name of entity";
            say("Could not change name of entity");
            allOkay = false;
        }
        opc->commit(e);
    }
    return allOkay;
}

void ICubClient::commitAgent()
{
    if (opc->isConnected())
        opc->commit(this->icubAgent);
}

std::string ICubClient::getPartnerName(bool verbose)
{
    string partnerName = "";
    list<shared_ptr<Entity>> lEntities = opc->EntitiesCacheCopy();
    for (auto& entity : lEntities) {
        if (entity->entity_type() == ICUBHRI_OPC_ENTITY_AGENT) {
            Agent* a = dynamic_cast<Agent*>(entity.get());
            //We assume kinect can only recognize one skeleton at a time
            if (a && a->m_present == 1.0 && a->name() != "icub") {
                partnerName = a->name();
                if (verbose) {
                    yInfo() << "Partner found: name =" << partnerName;
                }
                return partnerName;
            }
        }
    }
    if (verbose) {
        yWarning() << "No partner present was found!";
    }
    return partnerName;
}

yarp::sig::Vector ICubClient::getPartnerBodypartLoc(std::string sBodypartName){

    Vector vLoc;

    //we extract the coordinates of a specific bodypart of the partner and we look with ARE
    string partnerName = getPartnerName();
    if (partnerName == ""){
        yWarning() << "[iCubclient] Called getPartnerBodypartLoc :No partner present was found: cannot look at his/her ";
        return vLoc;
    }


    if (Agent *oPartner = dynamic_cast<Agent*>(opc->getEntity(partnerName, true))){
        if (oPartner->m_present == 1.0){
            if (oPartner->m_body.m_parts.find(sBodypartName) != oPartner->m_body.m_parts.end()){
                vLoc = oPartner->m_body.m_parts[sBodypartName];
                yDebug() << "The bodypart " << sBodypartName << "of the agemt " << partnerName << " is  at position " << vLoc.toString();
                return vLoc;
            }
            else {
                yError() << "[iCubClient] Called getPartnerBodypartLoc() on an unavalid bodypart (" << sBodypartName << ")";
                return vLoc;
            }
        }
        else {
            yError() << "[iCubClient] Called getPartnerBodypartLoc() on a non-present agent (" << partnerName << ")";
            return vLoc;
        }
    }

    return vLoc;
}

bool ICubClient::lookAtPartner()
{
    return look(getPartnerName());
}

bool ICubClient::babbling(const string &bpName, const string &babblingLimb, double train_dur)
{
    //check the subsystem is running
    if (subSystems.find("babbling") != subSystems.end()){

        //extract the bodypart with the name
        Entity *target = opc->getEntity(bpName, true);
        if (!target->isType(ICUBHRI_OPC_ENTITY_BODYPART))
        {
            yError() << "[iCubClient] Called babbling() on a unallowed entity: \"" << bpName << "\"";
            return false;
        }

        Bodypart *bp = dynamic_cast<Bodypart*>(target);
        if(bp) {
            int jointNumber = bp->m_joint_number;
            if (jointNumber == -1){
                yError() << "[iCubClient] Called babbling() on " << bpName << " which have no joint number linked to it\"";
                return false;
            }

            return ((SubSystem_babbling*)subSystems["babbling"])->babbling(jointNumber, babblingLimb, train_dur);
        } else {
            yError() << "Could not cast" << target << "to Bodypart";
            return false;
        }
    }

    yError() << "Error, babbling is not running...";
    return false;
}

bool ICubClient::babbling(int jointNumber, const string &babblingLimb, double train_dur)
{
    if (subSystems.find("babbling") != subSystems.end())
        return ((SubSystem_babbling*)subSystems["babbling"])->babbling(jointNumber, babblingLimb, train_dur);

    yError() << "Error, babbling is not running...";
    return false;
}


std::tuple<std::string, double> ICubClient::getHighestEmotion()
{
    double intensity = 0.0;
    string emotionName = "joy";

    //cout<<"EMOTIONS : "<<endl;
    for (map<string, double>::iterator d = this->icubAgent->m_emotions_intrinsic.begin(); d != this->icubAgent->m_emotions_intrinsic.end(); d++)
    {
        //cout<<'\t'<<d->first<<'\t'<<d->second<<endl;
        if (d->second > intensity)
        {
            emotionName = d->first;
            intensity = d->second;
        }
    }

    return std::make_tuple(emotionName, intensity);
}


bool ICubClient::say(const string &text, bool shouldWait)
{
    if (subSystems.find("speech") == subSystems.end())
    {
        yError() << "Impossible, speech is not running...";
        return false;
    }

    yDebug() << "iCub says" << text;
    ((SubSystem_Speech*)subSystems["speech"])->TTS(text, shouldWait);
    return true;
}

list<Action*> ICubClient::getKnownActions()
{
    this->actionsKnown.clear();
    list<Entity*> entities = opc->Entities(ICUBHRI_OPC_ENTITY_TAG, "==", ICUBHRI_OPC_ENTITY_ACTION);
    for (list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
    {
        actionsKnown.push_back(dynamic_cast<Action*>(*it));
    }
    return actionsKnown;
}


list<Object*> ICubClient::getObjectsInSight()
{
    list<Object*> inSight;
    opc->checkout();
    list<Entity*> allEntities = opc->EntitiesCache();
    for (list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); it++)
    {
        if ((*it)->isType(ICUBHRI_OPC_ENTITY_OBJECT))
        {
            Vector itemPosition = this->icubAgent->getSelfRelativePosition((dynamic_cast<Object*>(*it))->m_ego_position);

            //For now we just test if the object is in front of the robot
            if (itemPosition[0] < 0)
                inSight.push_back(dynamic_cast<Object*>(*it));
        }
    }
    return inSight;
}


list<Object*> ICubClient::getObjectsInRange()
{
    list<Object*> inRange;
    opc->checkout();
    list<Entity*> allEntities = opc->EntitiesCache();
    for (list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); it++)
    {
        if ((*it)->isType(ICUBHRI_OPC_ENTITY_OBJECT))
        {
            Object* o = dynamic_cast<Object*>(*it);
            if(o && o->m_present == 1.0) {
                Vector itemPosition = this->icubAgent->getSelfRelativePosition((dynamic_cast<Object*>(*it))->m_ego_position);
    
                if (isTargetInRange(itemPosition))
                    inRange.push_back(dynamic_cast<Object*>(*it));
            }
        }
    }
    return inRange;
}


bool ICubClient::isTargetInRange(const Vector &target) const
{
    bool isIn = ((target[0] > xRangeMin) && (target[0] < xRangeMax) &&
        (target[1] > yRangeMin) && (target[1]<yRangeMax) &&
        (target[2]>zRangeMin) && (target[2] < zRangeMax));

    return isIn;
}


SubSystem_agentDetector* ICubClient::getAgentDetectorClient()
{
    return getSubSystem<SubSystem_agentDetector>(SUBSYSTEM_AGENTDETECTOR);
}

SubSystem_babbling* ICubClient::getBabblingClient()
{
    return getSubSystem<SubSystem_babbling>(SUBSYSTEM_BABBLING);
}

SubSystem_IOL2OPC* ICubClient::getIOL2OPCClient()
{
    return getSubSystem<SubSystem_IOL2OPC>(SUBSYSTEM_IOL2OPC);
}

SubSystem_Recog* ICubClient::getRecogClient()
{
    return getSubSystem<SubSystem_Recog>(SUBSYSTEM_RECOG);
}

SubSystem_ARE* ICubClient::getARE()
{
    return getSubSystem<SubSystem_ARE>(SUBSYSTEM_ARE);
}

SubSystem_KARMA* ICubClient::getKARMA()
{
    return getSubSystem<SubSystem_KARMA>(SUBSYSTEM_KARMA);
}

SubSystem_Speech* ICubClient::getSpeechClient()
{
    // first, try to get SUBSYSTEM_SPEECH
    // if it's not available, fall back to SUBSYSTEM_SPEECH_ESPEAK
    SubSystem_Speech* s = getSubSystem<SubSystem_Speech>(SUBSYSTEM_SPEECH);
    if(s==nullptr) {
        return getSubSystem<SubSystem_Speech>(SUBSYSTEM_SPEECH_ESPEAK);
    } else {
        return s;
    }
}

SubSystem_SAM* ICubClient::getSAMClient()
{
    return getSubSystem<SubSystem_SAM>(SUBSYSTEM_SAM);
}

SubSystem_emotion* ICubClient::getEmotionClient()
{
    return getSubSystem<SubSystem_emotion>(SUBSYSTEM_EMOTION);
}
