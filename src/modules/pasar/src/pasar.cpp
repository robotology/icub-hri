// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
* Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project IST-270490
* Authors: Stéphane Lallée, Grégoire Pointeau
* email:   stephane.lallee@gmail.com, greg.pointeau@gmail.com
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

#include "pasar.h"

using namespace icubclient;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

/************************************************************************/
bool PasarModule::configure(yarp::os::ResourceFinder &rf)
{
    string moduleName = rf.check("name", Value("pasar")).asString().c_str();
    setName(moduleName.c_str());

    //Parameters
    pTopDownAppearanceBurst = rf.check("parameterTopDownAppearanceBurst",
                                       Value(3.)).asDouble();
    pTopDownDisappearanceBurst = rf.check("parameterTopDownDisappearanceBurst",
                                          Value(2.)).asDouble();
    pTopDownAccelerationCoef = rf.check("parameterTopDownAccelerationCoef",
                                        Value(0.1)).asDouble();
    pExponentialDecrease = rf.check("ExponentialDecrease",
                                    Value(0.9)).asDouble();
    pTopDownWaving = rf.check("pTopDownWaving",
                              Value(0.2)).asDouble();
    dBurstOfPointing = rf.check("pBurstOfPointing",
                                Value(0.2)).asDouble();
    dthresholdAppear = rf.check("thresholdAppear", Value(1.)).asDouble();
    dthresholdDisappear = rf.check("thresholdDisappear", Value(2.)).asDouble();
    thresholdPointing = rf.check("thresholdDistancePointing", Value(.5)).asDouble();

    //check for decrease
    if (pExponentialDecrease >= 1 || pExponentialDecrease <= 0.0)   pExponentialDecrease = 0.95;

    presentRightHand.first = false;
    presentRightHand.second = false;
    presentLeftHand.first = false;
    presentLeftHand.first = false;

    rightHandt1 = Vector(3, 0.0);
    rightHandt2 = Vector(3, 0.0);
    leftHandt1 = Vector(3, 0.0);
    leftHandt2 = Vector(3, 0.0);

    thresholdMovementAccelAgent = rf.check("thresholdMovementAccelAgent",
                                           Value(0.02)).asDouble();
    thresholdMovementAccelObject = rf.check("thresholdMovementAccelObject",
                                            Value(0.2)).asDouble();
    thresholdWaving = rf.check("thresholdWaving",
                               Value(0.02)).asDouble();
    thresholdSaliency = rf.check("thresholdSaliency",
                                 Value(0.005)).asDouble();

    bool isRFVerbose = true;
    iCub = new ICubClient(moduleName, "pasar", "pasar.ini", isRFVerbose);
    iCub->opc->isVerbose &= true;

    if (!iCub->connect())
    {
        yError() << "iCubClient : Some dependencies are not running...";
        return false;
    }

    checkPointing = rf.find("isPointing").asInt() == 1;
    checkWaving = rf.find("isWaving").asInt() == 1;

    isPointing = false;
    isWaving = false;

    yInfo() << " pointing: " << checkPointing;
    yInfo() << " waving: " << checkWaving;

    if (!handlerPort.open(("/" + moduleName + "/rpc").c_str())) {
        yError() << getName() << ": Unable to open port rpc";
        return false;
    }

    attach(handlerPort);                  // attach to port
    presentObjectsLastStep.clear();
    initTime = yarp::os::Time::now();
    initializeMapTiming();

    for (auto &it : OPCEntities)
    {
        yInfo() << "start: checking entity: " << it.second.o.name() << " opcPresence: " << it.second.o.m_present << ", pasar presence: " << it.second.present;
    }

    yInfo() << "\n \n" << "----------------------------------------------" << "\n \n" << moduleName << " ready ! \n \n ";


    return true;
}

/************************************************************************/
bool PasarModule::interruptModule()
{
    yDebug() << "Interrupt OPC";
    iCub->opc->interrupt();
    yDebug() << "Interrupt RPC port";
    handlerPort.interrupt();
    return true;
}

/************************************************************************/
bool PasarModule::close()
{
    if(iCub) {
        yDebug() << "Close iCub";
        iCub->close();
        delete iCub;
    }
    yDebug() << "Interrupt RPC";
    handlerPort.interrupt();
    yDebug() << "Close RPC";
    handlerPort.close();
    return true;
}

/************************************************************************/
bool PasarModule::respond(const Bottle& command, Bottle& reply)
{
    string helpMessage = string(getName().c_str()) +
            " commands are: \n" +
            "help \n" +
            "pointing on/off:  launch or stop pointing\n" +
            "waving on/off:  launch or stop waving\n" +
            "quit \n";

    reply.clear();
    reply.addString("ack");

    yInfo() << " rpc command received: " << command.toString();

    if (command.get(0).asString() == "quit") {
        reply.addString("quitting");
        return false;
    }
    else if (command.get(0).asString() == "help") {
        reply.addString(helpMessage.c_str());
    }
    else if (command.get(0).asString() == "pointing") {
        if (command.size() != 2)
        {
            reply.addString("error in PASAR: Bottle 'pointing' misses information (on/off)");
        }
        else
        {
            if (command.get(1).asString() == "off")
            {
                checkPointing = false;
                yInfo() << " stop pointing";
                reply.addString("ack");
                reply.addString("stop pointing");
            }
            else if (command.get(1).asString() == "on")
            {
                checkPointing = true;
                yInfo() << " start pointing";
                reply.addString("ack");
                reply.addString("start pointing");
            }
        }
    }
    else if (command.get(0).asString() == "waving") {
        if (command.size() != 2)
        {
            reply.addString("error in PASAR: Bottle 'waving' misses information (on/off)");
        }
        else
        {
            if (command.get(1).asString() == "off")
            {
                checkWaving = false;
                yInfo() << " stop waving";
                reply.addString("stop waving");
            }
            else if (command.get(1).asString() == "on")
            {
                checkWaving = true;
                yInfo() << " start waving";
                reply.addString("start waving");
            }
        }
    }
    else if (command.get(0).asString() == "set") {
        reply.addString("set");
        reply.addString("ir");
    }
    return true;
}

/***************************************************************************/
bool PasarModule::updateModule()
{
    iCub->opc->checkout();
    std::list<std::shared_ptr<icubclient::Entity>> entities = iCub->opc->EntitiesCacheCopy();

    presentLastSpeed = presentCurrentSpeed;
    presentCurrentSpeed.clear();

    for (auto &entity : entities)
    {
        if (entity->name() != "icub")
        {
            if (entity->isType(ICUBCLIENT_OPC_ENTITY_AGENT) || entity->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
            {
                Object * ob = dynamic_cast<Object*>(entity.get());
                if(ob)
                    OPCEntities[entity->opc_id()].o = *ob;
            }
        }
    }

    if (presentObjectsLastStep.size() > 0)
    {
        //Compute top down saliency (concept based)
        saliencyTopDown();
        if (checkPointing) saliencyPointing();
        if (checkWaving) saliencyWaving();

        //Leaky integrate
        saliencyLeakyIntegration();

        //Update the OPC values
        for (auto it = entities.begin(); it != entities.end(); it++)
        {
            if (OPCEntities.find((*it)->opc_id()) != OPCEntities.end())
            {
                Object* o = dynamic_cast<Object*>(iCub->opc->getEntity((*it)->name()));
                if(o) {
                    o->m_saliency = OPCEntities[(*it)->opc_id()].o.m_saliency;
                    iCub->opc->commit(o);
                }
            }
        }
    }
    else
    {
        yInfo("no objects present in the OPC");
    }

    presentObjectsLastStep = OPCEntities;

    return true;
}

/************************************************************************/
void PasarModule::saliencyTopDown()
{
    double now = yarp::os::Time::now() - initTime;
    //Add up the top down saliency
    for (auto &it : OPCEntities)
    {
        if (presentObjectsLastStep.find(it.first) != presentObjectsLastStep.end() && it.second.o.name() != "cursor_0" && it.second.o.name() != "icub")
        {
            //Objects appears/disappears
            bool appeared, disappeared;

            // If the object is absent:
            // if the lastTimeSeen was more than a threshold, the object dissapeared
            disappeared = (now - it.second.lastTimeSeen > dthresholdDisappear) && (it.second.o.m_present == 0.0) && it.second.present;
            if (disappeared){
                yInfo() << "DISAPPEARED since: " << now - it.second.lastTimeSeen;
            }

            // If the object is present:
            // if the lastTimeSeen was more than a threshold, the object appeared
            appeared = (now - it.second.lastTimeSeen > dthresholdAppear) && (it.second.o.m_present == 1.0) && !it.second.present;
            if (appeared){
                yInfo() << "APPEARED since: " << now - it.second.lastTimeSeen;
            }

            if (it.second.o.m_present == 1.0) it.second.lastTimeSeen = now;

            //instantaneous speed/acceleration
            Vector lastPos = presentObjectsLastStep[it.first].o.m_ego_position;
            Vector currentPos = it.second.o.m_ego_position;
            presentCurrentSpeed[it.first] = pair<double, double>(lastPos[0] - currentPos[0], lastPos[1] - currentPos[1]);

            double acceleration = sqrt(pow(presentCurrentSpeed[it.first].first - presentLastSpeed[it.first].first, 2.) + pow(presentCurrentSpeed[it.first].second - presentLastSpeed[it.first].second, 2.));

            OPCEntities[it.first].acceleration = acceleration;

            //Use the world model (CONCEPTS <=> RELATIONS) to modulate the saliency
            if (appeared)
            {
                it.second.o.m_saliency += pTopDownAppearanceBurst;
                yInfo() << "\t\t APPEARANCE OF: " << it.second.o.name();
                it.second.present = true;
            }

            if (disappeared)
            {
                it.second.o.m_saliency += pTopDownDisappearanceBurst;
                yInfo() << "\t\t DISAPPEARANCE OF: " << it.second.o.name();
                it.second.present = false;
            }

            if (it.second.o.entity_type() == ICUBCLIENT_OPC_ENTITY_AGENT){
                if (acceleration > thresholdMovementAccelAgent)
                {
                    it.second.o.m_saliency += pTopDownAccelerationCoef;
                    yInfo() << " moving agent:" << it.second.o.name() << " salience : " << it.second.o.m_saliency << " acceleration: " << acceleration;
                }

            }
            else {
                if (acceleration > thresholdMovementAccelObject)
                {
                    it.second.o.m_saliency += pTopDownAccelerationCoef;
                    yInfo() << " moving object:" << it.second.o.name() << " salience : " << it.second.o.m_saliency << " acceleration: " << acceleration;
                }
            }
        }
    }
}

/************************************************************************/
void PasarModule::saliencyNormalize()
{
    //Get the max
    map< int, ObjectModel >::iterator mostSalientObject = OPCEntities.begin();

    for (map< int, ObjectModel >::iterator it = OPCEntities.begin(); it != OPCEntities.end(); it++)
    {
        if (it->second.o.m_saliency > mostSalientObject->second.o.m_saliency)
            mostSalientObject = it;
    }

    //Normalize
    double maxS = mostSalientObject->second.o.m_saliency;
    if (maxS == 0) maxS = 1.;
    for (map< int, ObjectModel >::iterator it = OPCEntities.begin(); it != OPCEntities.end(); it++)
    {
        it->second.o.m_saliency /= maxS;
    }
}

/************************************************************************/
void PasarModule::saliencyLeakyIntegration()
{
    for (auto &it : OPCEntities)
    {
        if (it.second.o.name() != "" && it.second.o.m_present != 0.5)
        {
            it.second.o.m_saliency *= pExponentialDecrease;
        }
        if (it.second.o.m_saliency < thresholdSaliency)
            it.second.o.m_saliency = 0.0;
    }
}

/************************************************************************/
double PasarModule::getPeriod()
{
    return 0.1;
}


bool PasarModule::saliencyPointing()
{
    bool startPointing = false;
    bool stopPointing = false;

    bool wasPresent = false;
    Agent *ag = nullptr;
    // founding the agent:
    Vector vec;
    for (auto &it : OPCEntities){
        if (it.second.o.entity_type() == ICUBCLIENT_OPC_ENTITY_AGENT
                && it.second.present
                && (it.second.o.name() != "iCub")
                && (it.second.o.name() != "icub")){
            ag = dynamic_cast<Agent*>(iCub->opc->getEntity(it.second.o.name()));
            vec = ag->m_body.m_parts["handRight"];
            wasPresent = true;
        }
    }


    if (!ag || !wasPresent){
        yInfo() << " in PASAR:saliencyPointing no human agent present";
        return false;
    }

    double now = Time::now() - initTime;

    double x = vec[0];
    double y = vec[1];
    double z = vec[2];

    double closest = thresholdPointing;
    string objectPointed = "none";
    bool pointingNow = false;
    for (auto &it : OPCEntities)
    {
        if (it.second.o.name() != ag->name() && it.second.present)
        {
            double distance;

            distance = sqrt(
                        (x - it.second.o.m_ego_position[0])*(x - it.second.o.m_ego_position[0]) +
                        (y - it.second.o.m_ego_position[1])*(y - it.second.o.m_ego_position[1]) +
                        (z - it.second.o.m_ego_position[2])*(z - it.second.o.m_ego_position[2])
                    );

            if (distance < closest)
            {
                closest = distance;
                objectPointed = it.second.o.name();
                pointingNow = true;
            }
        }
    }

    stopPointing = (now - lastTimePointing > dthresholdDisappear) && isPointing && !pointingNow;
    startPointing = (now - lastTimePointing > dthresholdAppear) && !isPointing && pointingNow;

    if (stopPointing){
        yInfo() << "\t\t STOP POINTING ";
        isPointing = false;
    }

    if (startPointing){
        yInfo() << "\t\t POINTING START " << objectPointed;
        isPointing = true;
    }
    if (pointingNow){
        lastTimePointing = now;
        yInfo() << " pointed object is: \t" << objectPointed;
        for (auto &it : OPCEntities)
        {
            if (it.second.o.name() == objectPointed)
            {
                it.second.o.m_saliency += dBurstOfPointing;
            }
        }
    }
    return pointingNow;
}



bool PasarModule::saliencyWaving()
{
    bool wasPresent = false;
    Agent *ag = nullptr;
    // find the agent:
    for (auto &it : OPCEntities){
        if (it.second.o.entity_type() == ICUBCLIENT_OPC_ENTITY_AGENT
                && it.second.present
                && (it.second.o.name() != "iCub")
                && (it.second.o.name() != "icub")){
            ag = dynamic_cast<Agent*>(iCub->opc->getEntity(it.second.o.name()));
            wasPresent = true;
        }
    }

    if (!wasPresent){
        yInfo() << " in PASAR:saliencyWaving no human agent present";
        return false;
    }
    double dAccelLeft;
    double dAccelRight;
    bool wavingNow = false;

    if (!ag || (ag->m_present != 1.0))
    {
        presentRightHand.first = presentRightHand.second;
        presentLeftHand.first = presentLeftHand.second;

        presentRightHand.second = false;
        presentLeftHand.second = false;

        return false;
    }
    else {
        Vector vecRight = ag->m_body.m_parts["handRight"];
        Vector vecLeft = ag->m_body.m_parts["handLeft"];

        Vector speedRightt1(3);
        Vector speedRightt2(3);

        Vector speedLeftt1(3);
        Vector speedLeftt2(3);

        Vector accelRight(3);
        Vector accelLeft(3);


        speedRightt2[0] = (rightHandt1[0] - rightHandt2[0]);
        speedRightt2[1] = (rightHandt1[1] - rightHandt2[1]);
        speedRightt2[2] = (rightHandt1[2] - rightHandt2[2]);

        speedRightt1[0] = (vecRight[0] - rightHandt1[0]);
        speedRightt1[1] = (vecRight[1] - rightHandt1[1]);
        speedRightt1[2] = (vecRight[2] - rightHandt1[2]);

        accelRight[0] = (speedRightt1[0] - speedRightt2[0]);
        accelRight[1] = (speedRightt1[1] - speedRightt2[1]);
        accelRight[2] = (speedRightt1[2] - speedRightt2[2]);

        speedLeftt1[0] = (vecLeft[0] - leftHandt1[0]);
        speedLeftt1[1] = (vecLeft[1] - leftHandt1[1]);
        speedLeftt1[2] = (vecLeft[2] - leftHandt1[2]);

        speedLeftt2[0] = (leftHandt1[0] - leftHandt2[0]);
        speedLeftt2[1] = (leftHandt1[1] - leftHandt2[1]);
        speedLeftt2[2] = (leftHandt1[2] - leftHandt2[2]);

        accelLeft[0] = (speedLeftt1[0] - speedLeftt2[0]);
        accelLeft[1] = (speedLeftt1[1] - speedLeftt2[1]);
        accelLeft[2] = (speedLeftt1[2] - speedLeftt2[2]);

        // get the norm of the accel vector
        dAccelLeft = sqrt(accelLeft[0] * accelLeft[0] + accelLeft[1] * accelLeft[1] + accelLeft[2] * accelLeft[2]);
        dAccelRight = sqrt(accelRight[0] * accelRight[0] + accelRight[1] * accelRight[1] + accelRight[2] * accelRight[2]);

        rightHandt2 = rightHandt1;
        rightHandt1 = vecRight;

        leftHandt2 = leftHandt1;
        leftHandt1 = vecLeft;

        double now = yarp::os::Time::now() - initTime;
        wavingNow = false;

        // if the acceleration is made on 3 consecutive frames
        if (dAccelRight > thresholdWaving && presentRightHand.first && presentRightHand.second){
            yInfo() << "\tagent is waving right hand lastTime was: " << now - lastTimeWaving;
            wavingNow = true;
        }
        if (dAccelLeft > thresholdWaving && presentLeftHand.first && presentLeftHand.second){
            yInfo() << "\tagent is waving left hand lastTime was: " << now - lastTimeWaving;
            wavingNow |= true;
        }

        bool startWaving = (now - lastTimeWaving > dthresholdAppear) && !isWaving && wavingNow;
        bool stopWaving = (now - lastTimeWaving > dthresholdDisappear) && isWaving && !wavingNow;


        if (startWaving){
            isWaving = true;
            yInfo() << "\t\t START WAVING";
        }
        if (stopWaving){
            isWaving = false;
            yInfo() << "\t\t STOP  WAVING";
        }

        if (wavingNow){
            ag->m_saliency += pTopDownWaving;
            lastTimeWaving = now;
            iCub->opc->commit(ag);
        }

        presentRightHand.first = presentRightHand.second;
        presentLeftHand.first = presentLeftHand.second;

        presentRightHand.second = true;
        presentLeftHand.second = true;
    }
    return wavingNow;
}




void PasarModule::initializeMapTiming()
{
    isWaving = false;
    isPointing = false;

    iCub->opc->checkout();
    std::list<std::shared_ptr<icubclient::Entity>> entities = iCub->opc->EntitiesCacheCopy();
    double now = yarp::os::Time::now() - initTime;
    OPCEntities.clear();

    lastTimePointing = now - 10;
    lastTimeWaving = now - 10;

    for (auto &entity : entities){

        if (entity->name() != "icub")
        {
            //!!! ONLY OBJECTS and AGENTS ARE TRACKED !!!

            if (entity->isType(ICUBCLIENT_OPC_ENTITY_AGENT) || entity->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
            {
                Object * ob = dynamic_cast<Object*>(entity.get());
                if(ob) {
                    OPCEntities[entity->opc_id()].o = *ob;
                    OPCEntities[entity->opc_id()].lastTimeSeen = (ob->m_present == 1.0) ? now : now - 10;
                    OPCEntities[entity->opc_id()].present = (ob->m_present == 1.0);
                }
            }
        }
    }
}

