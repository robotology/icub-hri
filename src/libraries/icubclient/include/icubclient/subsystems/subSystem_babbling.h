#ifndef SUBSYSTEM_BABBLING_H
#define SUBSYSTEM_BABBLING_H

/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Maxime Petit
 * email:   m.petit@imperial.ac.uk
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

#define SUBSYSTEM_BABBLING           "babbling"

#include <iostream>
#include "icubclient/subsystems/subSystem.h"

namespace icubclient{
/**
* \ingroup icubclient_subsystems
*
* SubSystem for babbling
*/
class SubSystem_babbling : public SubSystem
{
protected:
    virtual bool connect();

public:

    /**
    * Default constructor.
    * @param masterName stem-name used to open up ports.
    */
    SubSystem_babbling(const std::string &masterName);

    virtual void Close();

    /**
     * @brief Whole arm babbling
     * @param babblingLimb: The limb to babble. Should be "left_arm" or "right_arm"
     * @param duration: How long to babble for
     * @return true if successful
     */
    bool babblingArm(const std::string& babblingLimb, double duration = -1.0);

    /**
     * @brief Single joint babbling
     * @param jointNumber: Which joint to babble.
     * @param babblingLimb: The limb to babble. Should be "left_arm" or "right_arm"
     * @param duration: How long to babble for
     * @return true if successful
     */
    bool babbling(int jointNumber, const std::string& babblingLimb, double duration = -1.0);

};
}//Namespace

#endif // SUBSYSTEM_BABBLING_H
