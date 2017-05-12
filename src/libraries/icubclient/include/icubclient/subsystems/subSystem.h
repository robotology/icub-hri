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

#ifndef __ICUBCLIENT_SUBSYSTEM_H__
#define __ICUBCLIENT_SUBSYSTEM_H__

#define SUBSYSTEM               "abstract"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <yarp/os/RpcClient.h>

/**
* @defgroup icubclient_subsystems Subsystems
* \ingroup icubclient_libraries
*
* Abstract class to handle sub-systems of the icub-client (speech, action, etc...)
* and the relevant subclasses
*/

namespace icubclient{
    /**
    * \ingroup icubclient_subsystems
    *
    * Abstract class to handle sub-systems of the icub-client
    */
    class SubSystem
    {
    protected:
        std::string m_masterName;
        std::string m_type;
        yarp::os::RpcClient portRPC;

        virtual bool connect() = 0;

    public:
        SubSystem(const std::string &masterName) { m_masterName = masterName; m_type = SUBSYSTEM; }
        virtual ~SubSystem() {}
        /**
         * @brief Connects the subsystem to its "host"
         * @return True if successful, false otherwise
         */
        bool Connect() {
            return connect();
        }

        /**
        * Clean up resources.
        */
        virtual void Close() = 0;

        /**
         * @brief Return the type of the subsystem as string
         */
        std::string getType() const { return m_type; }
    };
}//Namespace
#endif


