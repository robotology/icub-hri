/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Stéphane Lallée
 * email:   stephane.lallee@gmail.com
 * website: http://ICUBCLIENT.upf.edu/
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * wysiwyd/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include <yarp/os/LogStream.h>
#include "wrdac/clients/icubClient.h"
#include "wrdac/subsystems/subSystem_agentDetector.h"
#include "wrdac/subsystems/subSystem_ARE.h"
#include "wrdac/subsystems/subSystem_babbling.h"
#include "wrdac/subsystems/subSystem_iol2opc.h"
#include "wrdac/subsystems/subSystem_speech.h"
#include "wrdac/subsystems/subSystem_recog.h"
#include "wrdac/subsystems/subSystem_KARMA.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace wysiwyd::wrdac;

ICubClient::ICubClient(const std::string &moduleName, const std::string &context, const std::string &clientConfigFile, bool isRFVerbose)
{
    yarp::os::ResourceFinder rfClient;
    rfClient.setVerbose(isRFVerbose);
    rfClient.setDefaultContext(context.c_str());
    rfClient.setDefaultConfigFile(clientConfigFile.c_str());
    rfClient.configure(0, NULL);

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
    if (rangeMin == NULL) rangeMin = new Bottle(defaultRangeMin);
    if (rangeMax == NULL) rangeMax = new Bottle(defaultRangeMax);
    xRangeMin = defaultRangeMin.get(0).asDouble(); xRangeMax = defaultRangeMax.get(0).asDouble();
    yRangeMin = defaultRangeMin.get(1).asDouble(); yRangeMax = defaultRangeMax.get(1).asDouble();
    zRangeMin = defaultRangeMin.get(2).asDouble(); zRangeMax = defaultRangeMax.get(2).asDouble();

    icubAgent = NULL;

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
        delete sIt->second;
    }

    opc->interrupt();
    opc->close();
    delete opc;

    closed = true;
}


void ICubClient::updateAgent()
{
    if (opc->isConnected())
    {
        if (this->icubAgent == NULL)
        {
            icubAgent = opc->addOrRetrieveEntity<Agent>("icub");
        }
        else
            opc->update(this->icubAgent);
    }
    opc->Entities(ICUBCLIENT_OPC_ENTITY_TAG, "==", ICUBCLIENT_OPC_ENTITY_ACTION);
}

bool ICubClient::changeName(Entity *e, const std::string &newName) {
    bool allOkay = true;
    if (e->entity_type() == "agent") {
        if (subSystems.find("agentDetector") == subSystems.end()) {
            say("Could not change name of default partner of agentDetector");
            yWarning() << "Could not change name of default partner of agentDetector";
            opc->changeName(e, newName);
            opc->commit(e);

            allOkay = false;
        }
        else {
            dynamic_cast<SubSystem_agentDetector*>(subSystems["agentDetector"])->pause();

            opc->changeName(e, newName);
            opc->commit(e);

            if (!dynamic_cast<SubSystem_agentDetector*>(subSystems["agentDetector"])->changeDefaultName(newName)) {
                say("could not change default name of partner");
                yError() << "[SubSystem_agentDetector] Could not change default name of partner";
                allOkay = false;
            }

            dynamic_cast<SubSystem_agentDetector*>(subSystems["agentDetector"])->resume();
        }
        if (subSystems.find("recog") != subSystems.end()) {
            dynamic_cast<SubSystem_Recog*>(subSystems["recog"])->setSpeakerName(newName);
        }
    }
    else if (e->entity_type() == "object") {
        if (subSystems.find("iol2opc") == subSystems.end()) {
            say("iol2opc not reachable, did not change object name");
            yWarning() << "iol2opc not reachable, did not change object name";
            opc->changeName(e, newName);
            opc->commit(e);

            allOkay = false;
        }
        else {
            string oldName = e->name();
            if (!dynamic_cast<SubSystem_IOL2OPC*>(subSystems["iol2opc"])->changeName(oldName, newName)) {
                yError() << "iol2opc did not change name successfully";
                say("iol2opc did not change name successfully");
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

bool ICubClient::home(const string &part)
{
    SubSystem_ARE *are = getARE();
    if (are == NULL)
    {
        yError() << "[iCubClient] Called home() but ARE subsystem is not available.";
        return false;
    }

    return are->home(part);
}


bool ICubClient::grasp(const string &oName, const Bottle &options)
{
    Bottle opt(options);
    opt.addString("still"); // always avoid automatic homing after grasp

    SubSystem_ARE *are = getARE();
    if (are == NULL)
    {
        yError() << "[iCubClient] Called grasp() but ARE subsystem is not available.";
        return false;
    }

    return are->take(oName, opt);
}


bool ICubClient::release(const string &oLocation, const Bottle &options)
{
    Entity *target = opc->getEntity(oLocation, true);
    if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
    {
        yError() << "[iCubClient] Called release() on a unallowed location: \"" << oLocation << "\"";
        return false;
    }

    Object *oTarget = dynamic_cast<Object*>(target);
    if (oTarget->m_present != 1.0)
    {
        yWarning() << "[iCubClient] Called release() on an unavailable entity: \"" << oLocation << "\"";
        return false;
    }

    return release(oTarget->m_ego_position, options);
}


bool ICubClient::release(const Vector &target, const Bottle &options)
{
    SubSystem_ARE *are = getARE();
    if (are == NULL)
    {
        yError() << "[iCubClient] Called release() but ARE subsystem is not available.";
        return false;
    }

    if (isTargetInRange(target))
        return are->dropOn(target, options);
    else
    {
        yWarning() << "[iCubClient] Called release() on a unreachable location: (" << target.toString(3, 3).c_str() << ")";
        return false;
    }
}

bool ICubClient::waving(const bool sw) {
    SubSystem_ARE *are = getARE();
    if (are == NULL)
    {
        yError() << "[iCubClient] Called waving() but ARE subsystem is not available.";
        return false;
    }

    return are->waving(sw);
}


bool ICubClient::pointfar(const Vector &target, const Bottle &options, const std::string &sName)
{
    SubSystem_ARE *are = getARE();
    if (are == NULL)
    {
        yError() << "[iCubClient] Called pointfar() but ARE subsystem is not available.";
        return false;
    }

    Bottle opt(options);
    opt.addString("still"); // always avoid automatic homing after point
    return are->point(target, opt, sName);
}


bool ICubClient::point(const string &sName, const Bottle &options)
{
    Entity *target = opc->getEntity(sName, true);
    if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT) && !target->isType(ICUBCLIENT_OPC_ENTITY_BODYPART))
    {
        yWarning() << "[iCubClient] Called point() on a unallowed location: \"" << sName << "\"";
        return false;
    }

    Object *oTarget = dynamic_cast<Object*>(target);
    if(oTarget!=nullptr) {
        SubSystem_ARE *are = getARE();
        Vector target = oTarget->m_ego_position;
        are->selectHandCorrectTarget(const_cast<Bottle&>(options), target, sName);
        return pointfar(target, options, sName);
    } else {
        yError() << "[iCubClient] pointfar: Could not cast Entity to Object";
        return false;
    }
}


bool ICubClient::push(const string &oLocation, const Bottle &options)
{
    SubSystem_ARE *are = getARE();
    if (are == NULL)
    {
        yError() << "[iCubClient] Called push() but ARE subsystem is not available.";
        return false;
    }

    Bottle opt(options);
    opt.addString("still"); // always avoid automatic homing after point

    return are->push(oLocation, opt);
}


bool ICubClient::take(const std::string& sName, const Bottle &options)
{
    SubSystem_ARE *are = getARE();
    if (are == NULL)
    {
        yError() << "[iCubClient] Called take() but ARE subsystem is not available.";
        return false;
    }

    Bottle opt(options);
    opt.addString("still"); // always avoid automatic homing after point
    return are->take(sName, opt);
}

// KARMA

// Left push
bool ICubClient::pushKarmaLeft(const std::string &objName, const double &targetPosYLeft,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    if (opc->isConnected())
    {
        Entity *target = opc->getEntity(objName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
        {
            yWarning() << "[iCubClient] Called pushKarmaLeft() on a unallowed entity: \"" << objName << "\"";
            return false;
        }

        Object *oTarget = dynamic_cast<Object*>(target);
        if (oTarget->m_present != 1.0)
        {
            yWarning() << "[iCubClient] Called pushKarmaLeft() on an unavailable entity: \"" << objName << "\"";
            return false;
        }

        yInfo("[icubClient pushKarmaLeft] object %s position from OPC (no calibration): %s", oTarget->name().c_str(),
            oTarget->m_ego_position.toString().c_str());
        return pushKarmaLeft(objName, oTarget->m_ego_position, targetPosYLeft, armType, options);
    }
    else
    {
        yWarning() << "[iCubClient] There is no OPC connection";
        return false;
    }
}

bool ICubClient::pushKarmaLeft(const std::string &objName, const yarp::sig::Vector &objCenter, const double &targetPosYLeft,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pushKarmaLeft() but KARMA subsystem is not available.";
        return false;
    }
    return karma->pushAside(objName,objCenter, targetPosYLeft, 0, armType, options);
}

// Right push
bool ICubClient::pushKarmaRight(const std::string &objName, const double &targetPosYRight,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    if (opc->isConnected())
    {
        Entity *target = opc->getEntity(objName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
        {
            yWarning() << "[iCubClient] Called pushKarmaLeft() on a unallowed entity: \"" << objName << "\"";
            return false;
        }

        Object *oTarget = dynamic_cast<Object*>(target);
        if (oTarget->m_present != 1.0)
        {
            yWarning() << "[iCubClient] Called pushKarmaLeft() on an unavailable entity: \"" << objName << "\"";
            return false;
        }

        yInfo("[icubClient pushKarmaRight] object %s position from OPC (no calibration): %s", oTarget->name().c_str(),
            oTarget->m_ego_position.toString().c_str());
        return pushKarmaRight(objName, oTarget->m_ego_position, targetPosYRight, armType, options);
    }
    else
    {
        yWarning() << "[iCubClient] There is no OPC connection";
        return false;
    }
}

bool ICubClient::pushKarmaRight(const std::string &objName, const yarp::sig::Vector &objCenter, const double &targetPosYRight,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pushKarmaRight() but KARMA subsystem is not available.";
        return false;
    }
    return karma->pushAside(objName, objCenter, targetPosYRight, 180, armType, options);
}

// Front push
bool ICubClient::pushKarmaFront(const std::string &objName, const double &targetPosXFront,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    if (opc->isConnected())
    {
        Entity *target = opc->getEntity(objName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
        {
            yWarning() << "[iCubClient] Called pushKarmaFront() on a unallowed entity: \"" << objName << "\"";
            return false;
        }

        Object *oTarget = dynamic_cast<Object*>(target);
        if (oTarget->m_present != 1.0)
        {
            yWarning() << "[iCubClient] Called pushKarmaFront() on an unavailable entity: \"" << objName << "\"";
            return false;
        }

        yInfo("[icubClient pushKarmaFront] object %s position from OPC (no calibration): %s", oTarget->name().c_str(),
            oTarget->m_ego_position.toString().c_str());
        return pushKarmaFront(objName, oTarget->m_ego_position, targetPosXFront, armType, options);
    }
    else
    {
        yWarning() << "[iCubClient] There is no OPC connection";
        return false;
    }
}

bool ICubClient::pushKarmaFront(const std::string& objName, const yarp::sig::Vector &objCenter, const double &targetPosXFront,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pushKarmaFront() but KARMA subsystem is not available.";
        return false;
    }
    return karma->pushFront(objName, objCenter, targetPosXFront, armType, options);
}

// Pure push in KARMA
bool ICubClient::pushKarma(const yarp::sig::Vector &targetCenter, const double &theta, const double &radius,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pushKarma() but KARMA subsystem is not available.";
        return false;
    }
    return karma->push(targetCenter, theta, radius, options);
}

// Back pull
bool ICubClient::pullKarmaBack(const std::string &objName, const double &targetPosXBack,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    if (opc->isConnected())
    {
        Entity *target = opc->getEntity(objName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
        {
            yWarning() << "[iCubClient] Called pushKarmaFront() on a unallowed entity: \"" << objName << "\"";
            return false;
        }

        Object *oTarget = dynamic_cast<Object*>(target);
        if (oTarget->m_present != 1.0)
        {
            yWarning() << "[iCubClient] Called pushKarmaFront() on an unavailable entity: \"" << objName << "\"";
            return false;
        }

        yInfo("[icubClient pullKarmaBack] object %s position from OPC (no calibration): %s", oTarget->name().c_str(),
            oTarget->m_ego_position.toString().c_str());
        return pullKarmaBack(objName, oTarget->m_ego_position, targetPosXBack, armType, options);
    }
    else
    {
        yWarning() << "[iCubClient] There is no OPC connection";
        return false;
    }
}

bool ICubClient::pullKarmaBack(const std::string &objName,const yarp::sig::Vector &objCenter, const double &targetPosXBack,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pullKarmaBack() but KARMA subsystem is not available.";
        return false;
    }
    return karma->pullBack(objName,objCenter, targetPosXBack, armType, options);
}


// Pure pull (draw) in KARMA
bool ICubClient::drawKarma(const yarp::sig::Vector &targetCenter, const double &theta,
    const double &radius, const double &dist,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called drawKarma() but KARMA subsystem is not available.";
        return false;
    }
    return karma->draw(targetCenter, theta, radius, dist, options);
}

bool ICubClient::look(const yarp::sig::Vector &target, const yarp::os::Bottle &options,
                      const std::string& sName)
{
    if (SubSystem_ARE *are = getARE())
    {
        return are->look(target, options, sName);
    }

    yError() << "Error, ARE not running...";
    return false;
}

bool ICubClient::look(const string &target, const Bottle &options)
{
    if (SubSystem_ARE *are = getARE())
    {
        if (Object *oTarget = dynamic_cast<Object*>(opc->getEntity(target, true)))
            if (oTarget->m_present == 1.0)
                return are->look(oTarget->m_ego_position, options, oTarget->name());

        yWarning() << "[iCubClient] Called look() on an unavailable target: \"" << target << "\"";
        return false;
    }

    yError() << "Error, neither Attention nor ARE are running...";
    return false;
}

std::string ICubClient::getPartnerName(bool verbose)
{
    string partnerName = "";
    list<shared_ptr<Entity>> lEntities = opc->EntitiesCacheCopy();
    for (auto& entity : lEntities) {
        if (entity->entity_type() == "agent") {
            Agent* a = dynamic_cast<Agent*>(entity.get());
            //We assume kinect can only recognize one skeleton at a time
            if (a->m_present == 1.0 && a->name() != "icub") {
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


bool ICubClient::lookAtBodypart(const std::string &sBodypartName)
{
    if (SubSystem_ARE *are = getARE())
    {
        Vector vLoc;
        vLoc = getPartnerBodypartLoc(sBodypartName);
        if (vLoc.size() == 3){
            return are->look(vLoc, yarp::os::Bottle(), sBodypartName);
        }

        yWarning() << "[iCubClient] Called lookAtBodypart() on an unvalid/unpresent agent or bodypart (" << sBodypartName << ")";
        return false;
    }
    return false;
}

bool ICubClient::babbling(const string &bpName, const string &babblingLimb)
{
    //check the subsystem is running
    if (subSystems.find("babbling") != subSystems.end()){

        //extract the bodypart with the name
        Entity *target = opc->getEntity(bpName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_BODYPART))
        {
            yError() << "[iCubClient] Called babbling() on a unallowed entity: \"" << bpName << "\"";
            return false;
        }

        Bodypart *bp = dynamic_cast<Bodypart*>(target);
        int jointNumber = bp->m_joint_number;
        if (jointNumber == -1){
            yError() << "[iCubClient] Called babbling() on " << bpName << " which have no joint number linked to it\"";
            return false;
        }

        return ((SubSystem_babbling*)subSystems["babbling"])->babbling(jointNumber, babblingLimb);
    }

    yError() << "Error, babbling is not running...";
    return false;
}

bool ICubClient::babbling(int jointNumber, const string &babblingLimb)
{
    if (subSystems.find("babbling") != subSystems.end())
        return ((SubSystem_babbling*)subSystems["babbling"])->babbling(jointNumber, babblingLimb);

    yError() << "Error, babbling is not running...";
    return false;
}

bool ICubClient::babbling(const string &bpName, const string &babblingLimb, double train_dur)
{
    //check the subsystem is running
    if (subSystems.find("babbling") != subSystems.end()){

        //extract the bodypart with the name
        Entity *target = opc->getEntity(bpName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_BODYPART))
        {
            yError() << "[iCubClient] Called babbling() on a unallowed entity: \"" << bpName << "\"";
            return false;
        }

        Bodypart *bp = dynamic_cast<Bodypart*>(target);
        int jointNumber = bp->m_joint_number;
        if (jointNumber == -1){
            yError() << "[iCubClient] Called babbling() on " << bpName << " which have no joint number linked to it\"";
            return false;
        }

        return ((SubSystem_babbling*)subSystems["babbling"])->babbling(jointNumber, babblingLimb, train_dur);
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


void ICubClient::getHighestEmotion(string &emotionName, double &intensity)
{
    intensity = 0.0;
    emotionName = "joy";

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
}


bool ICubClient::say(const string &text, bool shouldWait, bool emotionalIfPossible, const std::string &overrideVoice, bool recordABM, std::string addressee)
{
    if (subSystems.find("speech") == subSystems.end())
    {
        yError() << "Impossible, speech is not running...";
        return false;
    }

    yDebug() << "iCub says" << text;
    ((SubSystem_Speech*)subSystems["speech"])->TTS(text, shouldWait, recordABM, addressee);
    return true;
}

list<Action*> ICubClient::getKnownActions()
{
    this->actionsKnown.clear();
    list<Entity*> entities = opc->Entities(ICUBCLIENT_OPC_ENTITY_TAG, "==", ICUBCLIENT_OPC_ENTITY_ACTION);
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
        if ((*it)->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
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
    //float sideReachability = 0.3f; //30cm on each side
    //float frontCloseReachability = -0.1f; //from 10cm in front of the robot
    //float frontFarReachability = -0.3f; //up to 30cm in front of the robot

    list<Object*> inRange;
    opc->checkout();
    list<Entity*> allEntities = opc->EntitiesCache();
    for (list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); it++)
    {
        if ((*it)->isType(ICUBCLIENT_OPC_ENTITY_OBJECT) && (dynamic_cast<Object*>(*it))->m_present == 1.0)
        {
            Vector itemPosition = this->icubAgent->getSelfRelativePosition((dynamic_cast<Object*>(*it))->m_ego_position);

            if (isTargetInRange(itemPosition))
                inRange.push_back(dynamic_cast<Object*>(*it));
        }
    }
    return inRange;
}


bool ICubClient::isTargetInRange(const Vector &target) const
{
    //cout<<"Target current root position is : "<<target.toString(3,3)<<endl;
    //cout<<"Range is : \n"
    //    <<"\t x in ["<<xRangeMin<<" ; "<<xRangeMax<<"]\n"
    //    <<"\t y in ["<<yRangeMin<<" ; "<<yRangeMax<<"]\n"
    //    <<"\t z in ["<<zRangeMin<<" ; "<<zRangeMax<<"]\n";

    bool isIn = ((target[0] > xRangeMin) && (target[0] < xRangeMax) &&
        (target[1] > yRangeMin) && (target[1]<yRangeMax) &&
        (target[2]>zRangeMin) && (target[2] < zRangeMax));
    //cout<<"Target in range = "<<isIn<<endl;

    return isIn;
}

SubSystem_IOL2OPC* ICubClient::getIOL2OPCClient()
{
    if (subSystems.find(SUBSYSTEM_IOL2OPC) == subSystems.end())
        return NULL;
    else
        return (SubSystem_IOL2OPC*)subSystems[SUBSYSTEM_IOL2OPC];
}

SubSystem_Recog* ICubClient::getRecogClient()
{
    if (subSystems.find(SUBSYSTEM_RECOG) == subSystems.end())
        return NULL;
    else
        return (SubSystem_Recog*)subSystems[SUBSYSTEM_RECOG];
}

SubSystem_ARE* ICubClient::getARE()
{
    if (subSystems.find(SUBSYSTEM_ARE) == subSystems.end())
        return NULL;
    else
        return (SubSystem_ARE*)subSystems[SUBSYSTEM_ARE];
}

SubSystem_Speech* ICubClient::getSpeechClient()
{
    if (subSystems.find(SUBSYSTEM_SPEECH) == subSystems.end())
    {
        if (subSystems.find(SUBSYSTEM_SPEECH_ESPEAK) == subSystems.end())
            return NULL;
        else
            return (SubSystem_Speech*)subSystems[SUBSYSTEM_SPEECH_ESPEAK];
    }
    else
        return (SubSystem_Speech*)subSystems[SUBSYSTEM_SPEECH];
}

SubSystem_KARMA* ICubClient::getKARMA()
{
    if (subSystems.find(SUBSYSTEM_KARMA) == subSystems.end())
        return NULL;
    else
        return (SubSystem_KARMA*)subSystems[SUBSYSTEM_KARMA];
}
