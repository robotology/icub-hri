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

private:
     /**
     * @brief Method to send iCub emotion to emotion yarp device.
     * @param emPair Pair with string emotion and string part
     * @return true if command was sent successful.
     */

    typedef std::map<std::string, std::string> StringMap;

    bool sendEmotion(std::pair<std::string, std::string> emPair);

    StringMap mapToEmotion = StringMap {
                                        {"neutral",      "neu"},
                                        {"talking",      "tal"},
                                        {"happy",        "hap"},
                                        {"sad",          "sad"},
                                        {"surprised",    "sur"},
                                        {"evil",         "evi"},
                                        {"angry",        "ang"},
                                        {"shy",          "shy"},
                                        {"cunning",      "cun"}
    };

    StringMap mapToPart =  StringMap {
                                        {"mouth",          "mou"},
                                        {"eyelids",        "eli"},
                                        {"left_eyebrow",   "leb"},
                                        {"right_eyebrow",  "reb"},
                                        {"all",            "all"}
    };

    std::pair<std::string, std::string> currentEmotionPair;

public:
    /**
    * Default constructor.
    * @param masterName stem-name used to open up ports.
    */
    SubSystem_emotion(const std::string &masterName);

    virtual void Close();

    /**
     * @brief Method to switch iCub emotion for different parts of face.
     * @param emotion String of emotion name
     * @param part String of part name - if "all", the whole face is affected
     * @return true if command was sent successful.
     */
    bool setEmotion(std::string emotion, std::string part="all");

    /**
     * @brief Method to get current iCub emotion
     * @return string with emotion name.
     */
    std::pair<std::string, std::string> getEmotion();

};
}//Namespace

#endif // SUBSYSTEM_EMOTION_H
