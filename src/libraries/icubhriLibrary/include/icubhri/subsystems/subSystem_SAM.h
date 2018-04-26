#ifndef SUBSYSTEM_SAM_H
#define SUBSYSTEM_SAM_H

/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Daniel Camilleri
 * email:   d.camilleri@sheffield.ac.uk
 * website: http://wysiwyd.upf.edu/
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * icub-hri/LICENSE
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#define SUBSYSTEM_SAM           "SAM"

#include <iostream>
#include "icubhri/subsystems/subSystem.h"

namespace icubhri{
/**
* \ingroup icubhri_subsystems
*
* SubSystem for SAM
*/
class SubSystem_SAM : public SubSystem
{
    friend class ICubClient;

protected:
    virtual bool connect();

public:
    bool attentionSAM;
    /**
    * Default constructor.
    * @param masterName stem-name used to open up ports.
    */
    SubSystem_SAM(const std::string &masterName);

    virtual void Close();

    /**
     * @brief Method to switch attention modes for SAM. 
     * @param mode Allowed modes are `stop` and `continue`.
     * @return true if command was send successful.
     */
    bool attentionModulation(const std::string &mode);

    /**
     * @brief Method to trigger a classification from SAM.
     * @param model Model name to trigger a classification for
     * @return Bottle with success as string as first value and classification as std::string as second value
     */
    yarp::os::Bottle askXLabel(const std::string &model);

};
}//Namespace

#endif // SUBSYSTEM_SAM_H
