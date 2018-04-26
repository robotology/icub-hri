/*
* Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
* Authors: Gregoire Pointeau, Maxime Petit
* email:   gregoire.pointeau@inserm.fr, m.petit@imperial.ac.uk
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

#ifndef __ICUBHRI_SUBSYSTEM_RECOG_H__
#define __ICUBHRI_SUBSYSTEM_RECOG_H__

#define SUBSYSTEM_RECOG        "recog"

#include "icubhri/subsystems/subSystem.h"

#include <iostream>

namespace icubhri{
/**
* \ingroup icubhri_subsystems
*
* Abstract subSystem for speech recognizer
*/
class SubSystem_Recog : public SubSystem
{
protected:
    virtual bool connect();
    yarp::os::RpcClient ears_port; /**< Port to /ears/rpc */

public:
    /**
    * Default constructor.
    * @param masterName stem-name used to open up ports.
    */
    SubSystem_Recog(const std::string &masterName);

    virtual void Close();

    /**
     * @brief Send a command to ears whether it should listen to the speechRecognizer
     * @param on: If true, ears will take control of speechRecognizer
     */
    void enableEars(bool on);

    /**
     * @brief Send a command to ears that it should stop listening to the speechRecognizer, and wait until it hands back control
     */
    void waitForEars();


    /**
     * @brief Interrupt the speech recognizer. This can be used to faster get back control of the speech recognizer if it is used by another
     * module currently (e.g. ears).
     * @return
     */
    bool interruptSpeechRecognizer();

    /**
    * From one grxml grammar, return the sentence recognized for one timeout
    * This is not supported, use SubSystem_Recog::recogFromGrammarLoop() instead!
    */
    yarp::os::Bottle recogFromGrammar(std::string &sInput);

    /**
    *   From one grxml grammar, return the first non-empty sentence recognized
    *   can last for several timeout (by default 50)
    * @param sInput: the grxml grammar in string form
    * @param iLoop: Maximum number of loops before giving up to recognize input. If -1, loop indefinitely
    * @param keepEarsEnabled: Whether ears should stay in control. This should only be set to true by ears itself
    * @param keepEarsDisabledAfterRecog: If false, ears will be enabled again after an input was recognized. This can be set to true if a module wants
    * to recognize several commands in a row without ever handing back control to ears.
    */
    yarp::os::Bottle recogFromGrammarLoop(std::string sInput, int iLoop = 50, bool keepEarsEnabled = false, bool keepEarsDisabledAfterRecog = false);

};
}//Namespace
#endif

