/*
* Copyright(C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT - 612139
* Authors: Nguyen Dong Hai Phuong
* email : phuong.nguyen@iit.it
* Permission is granted to copy, distribute, and / or modify this program
* under the terms of the GNU General Public License, version 2 or any
* later version published by the Free Software Foundation.
*
* A copy of the license can be found at
* icub-client / license / gpl.txt
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU General
* Public License for more details
*/

#include <yarp/os/all.h>
#include "icubclient/clients/icubClient.h"

int main() {
    yarp::os::Network yarp;

    icubclient::ICubClient iCub("KARMA_Simple");
    if(!iCub.connect()) { // connect to subsystems
        return -1;
    }

    // object name as recognized by object recognition
    std::string objectName = "octopus";
    double targetPositionX = -0.45;

    iCub.home();
    bool ok = iCub.pushKarmaFront(objectName, targetPositionX);
    yInfo()<<(ok?"Pushed successfully":"Failed");

    return 0;
}
