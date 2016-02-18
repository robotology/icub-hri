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
* wysiwyd/license/gpl.txt
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details
*/

#ifndef __EFAA_SUBSYSTEM_LRH_H__
#define __EFAA_SUBSYSTEM_LRH_H__

#define SUBSYSTEM_LRH        "lrh"

#include "wrdac/subsystems/subSystem.h"
#include "wrdac/subsystems/subSystem_ABM.h"

#include <iostream>

namespace wysiwyd{
    namespace wrdac{

        /**
        * \ingroup wrdac_clients
        *
        * Abstract subSystem for speech recognizer
        */
        class SubSystem_LRH : public SubSystem
        {
        protected:
            virtual bool connect() {
                // paste master name of 
                if (yarp::os::Network::isConnected(portRPC.getName(), "/lrh/rpc"))
                {
                    return true;
                }
                else
                {
                    return yarp::os::Network::connect(portRPC.getName(), "/lrh/rpc");
                }
            }
            SubSystem_ABM* SubABM;
            bool ABMconnected() {
                bool b = SubABM->Connect() && bForwardABM;
                yInfo() << (b ? "LRH connected to ABM" : "LRH not connected to ABM");
                return b;
            }
            bool SAMconnected() {
                if (yarp::os::Network::isConnected(portSAM.getName(), "/sam/rpc")){
                    return true;
                }
                else {
                    return yarp::os::Network::connect(portSAM.getName(), "/sam/rpc");
                }
            }

        public:
            bool bForwardABM;
            std::string interlocutor;
            std::string narrator;

            yarp::os::Port portRPC;
            yarp::os::Port portSAM;

            SubSystem_LRH(const std::string &masterName) : SubSystem(masterName){
                portRPC.open(("/" + m_masterName + "/lrh:o").c_str());
                portSAM.open(("/" + m_masterName + "/lrh:o:sam").c_str());
                SAMconnected();
                interlocutor = "partner";
                narrator = "iCub";
                m_type = SUBSYSTEM_LRH;
                bForwardABM = true;
                SubABM = new SubSystem_ABM(m_masterName + "/lrh");
            }


            virtual void Close() {
                portRPC.interrupt();
                portRPC.close();
                SubABM->Close();
            }


            std::string meaningToSentence(std::string sInput)
            {
                yarp::os::Bottle bMessenger,
                    bReturn;
                bMessenger.addString("production");
                bMessenger.addString(sInput);

                if (SAMconnected()){
                    std::istringstream iss(sInput);
                    std::string word;
                    int ii = 0;
                    int iSentence = 0;
                    yarp::os::Bottle toSAM;
                    while (iss >> word && word != "<o>") {
                        if (word == ","){
                            ii = 1;
                            iSentence++;
                        }
                        else{
                            if (iSentence != 0){
                                yarp::os::Bottle bTemp;
                                switch (ii){
                                case 1: {bTemp.addString("P" + std::to_string(iSentence)); break; }
                                case 2: {bTemp.addString("A" + std::to_string(iSentence)); break; }
                                case 3: {bTemp.addString("O" + std::to_string(iSentence)); break; }
                                case 4: {bTemp.addString("R" + std::to_string(iSentence)); break; }
                                }
                                bTemp.addString(word);
                                toSAM.addList() = bTemp;
                            }
                            ii++;
                        }
                    }
                    portSAM.write(toSAM);
                    std::cout << "to SAM is: " << toSAM.toString() << std::endl;
                }

                if (connect()){
                    portRPC.write(bMessenger, bReturn);
                    if (bReturn.size() == 2){
                        if (ABMconnected()){
                            std::list<std::pair<std::string, std::string> > lArgument;
                            lArgument.push_back(std::pair<std::string, std::string>(sInput, "meaning"));
                            lArgument.push_back(std::pair<std::string, std::string>(bReturn.get(1).toString(), "sentence"));
                            lArgument.push_back(std::pair<std::string, std::string>(m_masterName, "provider"));
                            lArgument.push_back(std::pair<std::string, std::string>(narrator, "speaker"));
                            lArgument.push_back(std::pair<std::string, std::string>(interlocutor, "addressee"));
                            SubABM->sendActivity("action", "production", "lrh", lArgument, true);
                        }
                        return bReturn.get(1).asString();
                    }
                    else{
                        yInfo() << "in subsystem lrh, error in meaningToSentence.";
                        return "none";
                    }
                }
                else {
                    yInfo() << "in subsystem lrh, LRH not connected: bypassing.";
                    return "none";
                }
            }

            std::string SentenceToMeaning(std::string sInput)
            {
                yarp::os::Bottle bMessenger,
                    bReturn;
                bMessenger.addString("meaning");
                bMessenger.addString(sInput);

                // parsing meaning to SAM

                if (connect()){
                    portRPC.write(bMessenger, bReturn);
                    if (bReturn.size() == 2){
                        if (ABMconnected()){
                            std::list<std::pair<std::string, std::string> > lArgument;
                            lArgument.push_back(std::pair<std::string, std::string>(sInput, "sentence"));
                            lArgument.push_back(std::pair<std::string, std::string>(bReturn.get(1).toString(), "meaning"));
                            lArgument.push_back(std::pair<std::string, std::string>(m_masterName, "provider"));
                            lArgument.push_back(std::pair<std::string, std::string>("iCub", "addressee"));
                            lArgument.push_back(std::pair<std::string, std::string>(interlocutor, "speaker"));
                            SubABM->sendActivity("action", "comprehension", "lrh", lArgument, true);
                        }

                        if (SAMconnected()){
                            std::stringstream iss(bReturn.get(1).toString());
                            std::string word;
                            yarp::os::Bottle toSAM;
                            std::vector<std::string> allElement;
                            std::cout << "iss " << iss.str() << std::endl;
                            // split by spaces

                            while (iss >> word)
                            {
                                std::vector<std::string> tmp = split(word, ',');
                                for (auto &el : tmp){
                                    allElement.push_back(el);
                                }
                            }

                            if (allElement.size() % 2 == 0)
                            {
                                unsigned int iNbElm = allElement.size() / 2;
                                for (unsigned int ii = 0; ii < iNbElm; ii++){
                                    yarp::os::Bottle bTemp;
                                    bTemp.addString(allElement[ii]);
                                    bTemp.addString(allElement[ii + iNbElm]);
                                    toSAM.addList() = bTemp;
                                }
                            }
                            else{
                                std::cout << "in lrh: sentenceToMeaning::forwardSAM problem in the size of meaning" << std::endl;
                            }
                            portSAM.write(toSAM);
                        }


                        return bReturn.get(1).asString();
                    }
                    else{
                        yInfo() << "in subsystem lrh, error in sentenceToMeaning.";
                        return "none";
                    }
                }
                else {
                    yInfo() << "in subsystem lrh, LRH not connected: bypassing.";
                    return "none";
                }
            }

            std::string SentenceFromPAORSimple(std::string P1, std::string A1, std::string O1 = "none", std::string R1 = "none"){
                std::ostringstream osMeaning;
                std::ostringstream osPAOR;
                int iOCW = 2;  // by default only A1 and P1 need to be fed;
                osMeaning << " , " << P1 << " " << A1 << " ";
                osPAOR << " <o> [_-_-_-_-_-_-_-_][A-P";
                if (O1 != "none"){
                    iOCW++;
                    osMeaning << O1 << " ";
                    osPAOR << "-O";
                }
                if (R1 != "none"){
                    iOCW++;
                    osMeaning << R1 << " ";
                    osPAOR << "-R";
                }

                for (unsigned int i = iOCW; i < 8; i++){
                    osPAOR << "-_";
                }
                osPAOR << "] <o>";

                osMeaning << osPAOR.str();

                if (connect()){
                    return meaningToSentence(osMeaning.str());
                }
                else {
                    std::ostringstream osRet;
                    osRet << A1 << " " << P1 << " ";
                    if (O1 != "none")  osRet << "the " << O1 << " ";
                    if (R1 != "none")  osRet << "to " << R1 << " ";
                    return osRet.str();
                }
            }

            std::string SentenceFromPAORDouble(std::string P1, std::string A1, std::string P2, std::string A2, std::string O1 = "none", std::string R1 = "none", std::string O2 = "none", std::string R2 = "none"){
                std::ostringstream osMeaning;
                std::ostringstream osPAOR;
                int iOCW = 4;  // by default A1 A2 P1 P2 need to be fed;
                osMeaning << " , " << P1 << " " << A1 << " ";
                osPAOR << " <o> [_-_-_-_-_-_-_-_][A-P";
                if (O1 != "none"){
                    iOCW++;
                    osMeaning << O1 << " ";
                    osPAOR << "-O";
                }
                if (R1 != "none"){
                    iOCW++;
                    osMeaning << R1 << " ";
                    osPAOR << "-R";
                }

                // second sentence

                osMeaning << " , " << P2 << " " << A2 << " ";
                osPAOR << "-A-P";

                if (O2 != "none"){
                    iOCW++;
                    osMeaning << O2 << " ";
                    osPAOR << "-O";
                }
                if (R2 != "none"){
                    iOCW++;
                    osMeaning << R2 << " ";
                    osPAOR << "-R";
                }
                for (unsigned int i = iOCW; i < 8; i++){
                    osPAOR << "-_";
                }
                osPAOR << "] <o>";

                osMeaning << osPAOR.str();

                if (connect()){
                    return meaningToSentence(osMeaning.str());
                }
                else {
                    std::ostringstream osRet;
                    osRet << A1 << " " << P1 << " ";
                    if (O1 != "none")  osRet << "the " << O1 << " ";
                    if (R1 != "none")  osRet << "to " << R1 << " ";
                    osRet << "and ";
                    osRet << A2 << " " << P2 << " ";
                    if (O2 != "none")  osRet << "the " << O2 << " ";
                    if (R2 != "none")  osRet << "to " << R2 << " ";

                    return osRet.str();

                }
            }
            std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
                std::stringstream ss(s);
                std::string item;
                while (std::getline(ss, item, delim)) {
                    elems.push_back(item);
                }
                return elems;
            }


            std::vector<std::string> split(const std::string &s, char delim) {
                std::vector<std::string> elems;
                split(s, delim, elems);
                return elems;
            }
        };


    }
}
//Namespace



#endif


