/*
* Copyright(C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT - 612139
* Authors: Tobias Fischer, Maxime Petit (Based on ARE/KARMA iCubClient examples of Ugo Pattacini and Nguyen Dong Hai Phuong)
* email : t.fischer@imperial.ac.uk, m.petit@imperial.ac.uk
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

#include <iostream>
#include <iomanip>
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>
#include <yarp/math/SVD.h>
#include "icubclient/clients/icubClient.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::math;
using namespace icubclient;

int main()
{
    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError()<<"YARP network seems unavailable!";
        return -1;
    }

    ICubClient iCub("ARE_KARMAiCubClientExample","icubClient","example_ARE_KARMA.ini");

    if(!iCub.connect()) { // we connect to ARE, KARMA and OPC
        yError()<<"[ARE_KARMAiCubClientExample] ARE and/or KARMA and/or OPC seems unavailabe!";
        return -1;
    }

    ResourceFinder rfClient;
    rfClient.setVerbose(true);
    rfClient.setDefaultContext("icubClient");
    rfClient.setDefaultConfigFile("example_ARE_KARMA.ini");
    rfClient.configure(0, NULL);

    // object location in the iCub frame
    Vector x(3);
    x[0]=-0.25;
    x[1]=-0.20;
    x[2]=-0.05;

    string target;
    if (rfClient.check("target")) {
        target = rfClient.find("target").asString();
        yInfo("target name set to %s", target.c_str());
    } else {
        target = "octopus";
        yInfo("target name set to default, i.e. %s", target.c_str());
    }
    bool ok;

    iCub.home();                        // Home by using ARE
    yInfo()<<"try to pushFront with KARMA ...";
    ok = iCub.pushKarma(x,-90,0.1);     //WARNING: second argument is the desired x[0] that we want and should be less than original x[0]
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    iCub.home();                        // Home by using ARE
    x[1] =x[1] - 0.1;                   //offset on lateral to draw?
    yInfo()<<"try to pull with KARMA...";
    ok = iCub.drawKarma(x,0,0.1,0.1);   //draw of 0.1m but the objects is considered as -0.35 so final: -0.25
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);
    x[1] = x[1] + 0.1;                  //go back to original value



    //----------------------------> ARE related action
    iCub.home();                        // Home by using ARE
    yInfo()<<"try to point using ARE...";
    ok = iCub.point(x);              // automatic selection of the hand
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    iCub.home();                        // Home by using ARE
    yInfo()<<"try to take from side using ARE...";
    Bottle bOptions("side");            // param1: side
    ok = iCub.take(target, bOptions);    // automatic selection of the hand
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    iCub.home();                        // Home by using ARE
    yInfo()<<"try to take from above using ARE...";
    bOptions.clear();
    bOptions.addString("above");
    ok = iCub.take(target, bOptions);   // automatic selection of the hand
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    iCub.home();                        // Home by using ARE
    yInfo()<<"try to push using ARE...";
    ok = iCub.push(target);             // automatic selection of the hand
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    iCub.home();                        // Home by using ARE
    yInfo()<<"try to push away using ARE...";
    bOptions.clear();
    bOptions.addString("away");
    ok = iCub.push(target,bOptions);    // automatic selection of the hand
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);
    iCub.home();


    yInfo()<<"shutting down ... ";
    iCub.close();
    return 0;
}


