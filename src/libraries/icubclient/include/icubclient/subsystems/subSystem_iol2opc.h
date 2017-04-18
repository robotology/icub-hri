/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Tobias Fischer
 * email:   t.fischer@imperial.ac.uk
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

#ifndef SUBSYSTEM_IOL2OPC_H
#define SUBSYSTEM_IOL2OPC_H

#define SUBSYSTEM_IOL2OPC           "iol2opc"

#include <iostream>
#include "icubclient/subsystems/subSystem.h"

namespace icubclient{
/**
* \ingroup icubclient_subsystems
*
* SubSystem for iol2opc
*/
class SubSystem_IOL2OPC : public SubSystem
{
    friend class ICubClient;

protected:
    virtual bool connect();

    /**
     * @brief Change the name of an object
     * Internally used by ICubClient::changeName
     * @param old_name: Name of the object to be changed
     * @param new_name: New name of the object
     */
    bool changeName(const std::string &old_name, const std::string &new_name);

    yarp::os::RpcClient portRPC;

public:

    /**
    * Default constructor.
    * @param masterName stem-name used to open up ports.
    */
    SubSystem_IOL2OPC(const std::string &masterName);

    virtual void Close();

    /**
     * @brief Pause the iol2opc object detection.
     */
    void pause();

    /**
     * @brief Resume the iol2opc object detection.
     */
    void resume();
};
}//Namespace

#endif // SUBSYSTEM_IOL2OPC_H
