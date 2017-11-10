#ifndef SUBSYSTEM_EMOTION_H
#define SUBSYSTEM_EMOTION_H

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
 * icub-client/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#define SUBSYSTEM_EMOTION           "emotion"

#include <iostream>
#include "icubclient/subsystems/subSystem.h"

namespace icubclient{
/**
* \ingroup icubclient_subsystems
*
* SubSystem for emotion
*/
class SubSystem_emotion : public SubSystem
{
    friend class ICubClient;

protected:
    virtual bool connect();

public:
    std::string currentEmotion;
    enum Part {mouth, eyelids, left_eyebrow, right_eyebrow, all};
    enum Emotion {neutral, talking, happy, sad, surprised, evil, angry, shy, cunning};

    /**
    * Default constructor.
    * @param masterName stem-name used to open up ports.
    */
    SubSystem_emotion(const std::string &masterName);

    virtual void Close();

    /**
     * @brief Method to switch iCub emotion for different parts of face.
     * @param emotion Selection from Emotion enum
     * @param part Selection from Part enum
     * @return true if command was sent successful.
     */
    bool setEmotion(Emotion emotion, Part part);

    /**
     * @brief Method to switch iCub emotions for whole face.
     * @param emotion Selection from Emotion enum
     * @return true if command was sent successful.
     */
    bool setEmotion(Emotion emotion);

};
}//Namespace

#endif // SUBSYSTEM_EMOTION_H
