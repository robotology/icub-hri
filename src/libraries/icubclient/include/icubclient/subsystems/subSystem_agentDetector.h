#ifndef SUBSYSTEM_AGENTDETECTOR_H
#define SUBSYSTEM_AGENTDETECTOR_H

/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Tobias Fischer
 * email:   t.fischer@imperial.ac.uk
 * website: http://wysiwyd.upf.edu/
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

#define SUBSYSTEM_AGENTDETECTOR           "agentDetector"

#include <iostream>
#include "icubclient/subsystems/subSystem.h"

namespace icubclient{
/**
* \ingroup icubclient_subsystems
*
* SubSystem for agentDetector
*/
class SubSystem_agentDetector : public SubSystem
{
    friend class ICubClient;

protected:
    virtual bool connect();

    /**
     * @brief Changes the name of the partner within agentDetector
     * Internally used by ICubClient::changeName
     * @param new_name
     * @return true if successful, false otherwise
     */

    bool changeDefaultName(std::string new_name);

    yarp::os::RpcClient portRPC;

public:

    /**
    * Default constructor.
    * @param masterName stem-name used to open up ports.
    */
    SubSystem_agentDetector(const std::string &masterName);

    virtual void Close();

    /**
     * @brief Pause the agentDetector. No skeleton information of the human will be updated.
     */
    void pause();

    /**
     * @brief The skeleton detection of agentDetector will be resumed.
     */
    void resume();
};
}//Namespace

#endif // SUBSYSTEM_AGENTDETECTOR_H
