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

#ifndef __GUIUPV2_H__
#define __GUIUPV2_H__

#include <string>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include "icubclient/clients/opcClient.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace icubclient;

class GuiUpdater: public RFModule
{
private:
    OPCClient *opc;
    Agent* iCub;
    RpcServer handlerPort;      //a port to handle messages
    RpcClient toGui;
    RpcClient toGuiBase;
    list<shared_ptr<Entity>> oldEntities;
    bool displaySkeleton;

public:
    bool configure(yarp::os::ResourceFinder &rf); // configure all the module parameters and return true if successful
    bool interruptModule();                       // interrupt, e.g., the ports 
    bool close();                                 // close and shut down the module
    bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);
    double getPeriod(); 
    bool updateModule();

    /**
     * @brief Send a request to the iCubGUI to remove all items
     */
    void resetGUI();

    /**
     * @brief Deletes an object with name `opcTag` from the OPC
     * @param opcTag - name of the object to be deleted
     * @param o - if this is an `Agent` instance, take care of removing all associated body parts
     */
    void deleteObject(const string &opcTag, Object* o = NULL);

    /**
     * @brief Adds an object to the iCubGUI. Takes care of reading the position, dimensions, color etc.
     * @param o - the object to be added
     */
    void addObject(Object* o);

    /**
     * @brief Adds an agent to the iCubGUI. Takes care of adding the body parts if `displaySkeleton` is true
     * @param a - the agent to be added
     */
    void addAgent(Agent* a);

    /**
     * @brief Adds the iCub the the iCubGUI
     * @param a - reference to the iCub agent
     */
    void addiCub(Agent* a);

    /**
     * @brief Checks whether an Entity can be displayed in the GUI
     * @param entity - Entity to be checked
     * @return true if it is an `Object` or a subtype of `Object`, false otherwise
     */
    bool isDisplayable(Entity* entity);
};

#endif
