/* 
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Stéphane Lallée
 * email:   stephane.lallee@gmail.com
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

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>
#include <iostream>
#include <string>
#include <map>
#include <list>

#include "guiUpdater.h"

using namespace std;
using namespace yarp;
using namespace yarp::sig;
using namespace yarp::os;
using namespace yarp::dev;


bool GuiUpdater::configure(yarp::os::ResourceFinder &rf)
{
    string moduleName = rf.check("name", Value("guiUpdater"), "module name (string)").asString().c_str();
    string opcName = rf.check("OPCname", Value("OPC"), "OPC name (string)").asString().c_str();

    setName(moduleName.c_str());

    displaySkeleton = rf.check("displaySkeletons");
    yInfo() << "Display skeleton status:" << displaySkeleton;

    opc = new OPCClient(getName().c_str());
    opc->connect(opcName);
    opc->isVerbose = false;
    if(opc->isConnected()) {
        iCub = opc->addOrRetrieveEntity<Agent>("icub");
    } else {
        yError() << "Could not connect to OPC, abort";
        return false;
    }

    //GUI Port
    toGui.open("/" + getName() + "/gui:o");
    resetGUI();

    //GUI Base Port
    toGuiBase.open("/" + getName() + "/guiBase:o");

    //RPC
    if (!handlerPort.open("/" + getName() + "/rpc")) {
        yError() << getName() << ": Unable to open port " << handlerPort.getName();
        return false;
    }
    attach(handlerPort);

    return true;
}

bool GuiUpdater::interruptModule()
{
    handlerPort.interrupt();
    opc->interrupt();
    toGui.interrupt();
    toGuiBase.interrupt();
    return true;
}

bool GuiUpdater::close()
{
    yDebug() << "close";
    handlerPort.interrupt();
    handlerPort.close();

    if(opc) {
        opc->close();
        delete opc;
    }

    toGui.interrupt();
    toGui.close();
    toGuiBase.interrupt();
    toGuiBase.close();

    yDebug() << "bye";

    return true;
}

bool GuiUpdater::respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply)
{  
    string helpMessage =  string(getName().c_str()) +
            " commands are: \n" +
            "reset \n" +
            "help \n" +
            "quit \n" ;

    reply.clear();

    if (command.get(0).asString()=="quit") {
        reply.addString("quitting");
        return false;
    }
    else if (command.get(0).asString()=="help") {
        yInfo() << helpMessage;
        reply.addString("ok");
        reply.addString(helpMessage);
    }
    else if (command.get(0).asString()=="reset") {
        yInfo() << "Reset";
        resetGUI();
        reply.addString("ok");
    }
    return true;
}

double GuiUpdater::getPeriod()
{
    return 0.1;
}

bool GuiUpdater::updateModule()
{
    if (opc->isConnected())
    {
        opc->checkout(); //Retrieve all entities

        //Display the iCub specifics
        addiCub(iCub);

        //Display the objects
        list<shared_ptr<Entity>> entities = opc->EntitiesCacheCopy();
        if (oldEntities.size() != entities.size()) {
            resetGUI();
        } else {
            list<shared_ptr<Entity>>::iterator e_new, e_old;
            for(e_new = entities.begin(), e_old = oldEntities.begin();
                e_new != entities.end() && e_old != oldEntities.end();
                e_new++, e_old++) {
                if((*e_new)->name() != (*e_old)->name()) {
                    resetGUI(); // Entities are the same, but a name has changed, so reset
                    break;
                }
            }
        }
        oldEntities = entities;

        for(auto& entity : entities) {
            if( isDisplayable(entity.get()) ) {
                Object* o = dynamic_cast<Object*>(entity.get());
                if(!o) {
                    yError() << "Could not cast " << entity->name();
                    continue;
                }

                ostringstream guiTag;
                guiTag << o->name() << "(" << o->opc_id() << ")";

                if (o->m_present==0.0) {
                    deleteObject(guiTag.str(), o);
                } else {
                    if (o->name() != "icub") {
                        if (o->isType(ICUBCLIENT_OPC_ENTITY_AGENT)) {
                            addAgent(dynamic_cast<Agent*>(o));
                        } else {
                            addObject(o);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool GuiUpdater::isDisplayable(Entity* entity)
{
    return entity->isType(ICUBCLIENT_OPC_ENTITY_OBJECT);
}

void GuiUpdater::deleteObject(const string &opcTag, Object* o)
{
    Bottle cmd;
    cmd.addString("delete");
    cmd.addString(opcTag.c_str());
    toGui.write(cmd);

    //Delete all the body parts
    if (o != NULL && o->entity_type() == ICUBCLIENT_OPC_ENTITY_AGENT && displaySkeleton)
    {
        unsigned int i = 0;
        Agent* a = dynamic_cast<Agent*>(o);
        while (i < a->m_body.m_parts.size()) // remove all body parts of the agent
        {
            ostringstream opcTagPart;
            opcTagPart << o->opc_id() << "_" << i;
            cmd.clear();
            cmd.addString("delete");
            cmd.addString(opcTagPart.str().c_str());
            toGui.write(cmd);
            i++;
        }
    }
}

void GuiUpdater::addAgent(Agent* o)
{
    if (displaySkeleton) // display all body parts
    {
        int i=0;
        for(auto& part : o->m_body.m_parts)
        {
            //Get the position of the object in the current reference frame of the robot (not the initial one)
            Vector inCurrentRootReference = iCub->getSelfRelativePosition(part.second);

            ostringstream opcTagPart;
            opcTagPart<<o->opc_id() << "_" << i;
            Bottle cmd;
            cmd.addString("object");
            cmd.addString(opcTagPart.str().c_str());

            //Body parts have fixed dimensions
            cmd.addDouble(0.05 *1000.0);    // dimX in [mm]
            cmd.addDouble(0.05 *1000.0);    // dimY in [mm]
            cmd.addDouble(0.05 *1000.0);    // dimZ in [mm]
            cmd.addDouble(inCurrentRootReference[0] *1000.0);        // posX in [mm]
            cmd.addDouble(inCurrentRootReference[1] *1000.0);        // posY in [mm]
            cmd.addDouble(inCurrentRootReference[2] *1000.0);        // posZ in [mm]
            cmd.addDouble(0);             // discard the orientation
            cmd.addDouble(0);             // "
            cmd.addDouble(0);             // "
            cmd.addInt((int)o->m_color[0]);            // color R
            cmd.addInt((int)o->m_color[1]);            // color G
            cmd.addInt((int)o->m_color[2]);            // color B
            cmd.addDouble(1);                     // alpha coefficient [0,1]
            toGui.write(cmd);
            i++;
        }
    }
    else // only display the head
    {
        Vector inCurrentRootReference = iCub->getSelfRelativePosition(o->m_ego_position);
        ostringstream opcTagPart;
        opcTagPart << o->name() << "(" << o->opc_id() << ")";
        Bottle cmd;
        cmd.addString("object");
        cmd.addString(opcTagPart.str().c_str());

        //Body parts have fixed dimensions
        cmd.addDouble(0.15 *1000.0);    // dimX in [mm]
        cmd.addDouble(0.15 *1000.0);    // dimY in [mm]
        cmd.addDouble(0.25 *1000.0);    // dimZ in [mm]
        cmd.addDouble(inCurrentRootReference[0]  *1000.0);        // posX in [mm]
        cmd.addDouble(inCurrentRootReference[1]  *1000.0);        // posY in [mm]
        cmd.addDouble(inCurrentRootReference[2]  *1000.0);        // posZ in [mm]
        cmd.addDouble(0 - iCub->m_ego_orientation[0]);             // discard the orientation
        cmd.addDouble(0 - iCub->m_ego_orientation[1]);             // "
        cmd.addDouble(0 - iCub->m_ego_orientation[2]);             // "
        cmd.addInt((int)o->m_color[0]);            // color R
        cmd.addInt((int)o->m_color[1]);            // color G
        cmd.addInt((int)o->m_color[2]);            // color B
        cmd.addDouble(1);                     // alpha coefficient [0,1]
        toGui.write(cmd);
    }
}

void GuiUpdater::addObject(Object* o)
{
    //Get the position of the object in the current reference frame of the robot (not the initial one)
    Vector inCurrentRootReference = iCub->getSelfRelativePosition(o->m_ego_position);

    ostringstream guiTag;
    guiTag << o->name() << "(" << o->opc_id() << ")";

    Bottle cmd;
    cmd.addString("object");
    cmd.addString(guiTag.str());

    cmd.addDouble(o->m_dimensions[0] *1000.0);    // dimX in [mm]
    cmd.addDouble(o->m_dimensions[1] *1000.0);    // dimY in [mm]
    cmd.addDouble(o->m_dimensions[2] *1000.0);    // dimZ in [mm]
    cmd.addDouble(inCurrentRootReference[0] *1000.0);        // posX in [mm]
    cmd.addDouble(inCurrentRootReference[1] *1000.0);        // posY in [mm]
    cmd.addDouble(inCurrentRootReference[2] *1000.0);        // posZ in [mm]
    cmd.addDouble(o->m_ego_orientation[0] - iCub->m_ego_orientation[0]);             // Deal with the object orientation that is moving with the base
    cmd.addDouble(o->m_ego_orientation[1] - iCub->m_ego_orientation[1]);             // "
    cmd.addDouble(o->m_ego_orientation[2] - iCub->m_ego_orientation[2]);             // "
    cmd.addInt((int)o->m_color[0]);            // color R
    cmd.addInt((int)o->m_color[1]);            // color G
    cmd.addInt((int)o->m_color[2]);            // color B
    cmd.addDouble(1);                     // alpha coefficient [0,1]
    toGui.write(cmd);
}

void GuiUpdater::addiCub(Agent* a)
{
    Bottle cmd;
    cmd.addDouble(a->m_ego_orientation[0]); //in opc we store rotation around x,y,z, which seems different of the iCubGUI
    cmd.addDouble(a->m_ego_orientation[1]);
    cmd.addDouble(a->m_ego_orientation[2]);
    cmd.addDouble(a->m_ego_position[0] * 1000.0);
    cmd.addDouble(a->m_ego_position[1] * 1000.0);
    cmd.addDouble(a->m_ego_position[2] * 1000.0);
    toGuiBase.write(cmd);
}

void GuiUpdater::resetGUI()
{
    Bottle cmd,reply;
    cmd.clear();
    reply.clear();
    cmd.addString("reset");
    toGui.write(cmd);
}
