/*
* Copyright(C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT - 612139
* Authors: Ugo Pattacini
* email : ugo.pattacini@iit.it
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
#include "wrdac/clients/icubClient.h"

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
        cout<<"YARP network seems unavailable!"<<endl;
        return -1;
    }

    ICubClient iCub("AREiCubClientExample","icubClient","example_ARE.ini");

    ResourceFinder rfClient;
    rfClient.setVerbose(true);
    rfClient.setDefaultContext("icubClient");
    rfClient.setDefaultConfigFile("example_ARE.ini");
    rfClient.configure(0, NULL);

    // we connect to both ARE and OPC
    if(!iCub.connect()) {
        yError()<<"[ARE_KARMAiCubClientExample] ARE and/or OPC seems unavailabe!";
        return -1;
    }

    string target;
    if (rfClient.check("target")) {
        target = rfClient.find("target").asString();
        yInfo("target name set to %s", target.c_str());
    } else {
        target = "octopus";
        yInfo("target name set to default, i.e. %s", target.c_str());
    }

    double radius=0.025;

    // object location in the iCub frame
    Vector x(3);
    x[0]=-0.35;
    x[1]=-0.05;
    x[2]=-0.05;

    iCub.home();
    yInfo()<<"pointing at the object ... ";
    Bottle options("right");    // force the use of the right hand
    iCub.pointfar(x, options);  // automatic selection of the hand
    Time::delay(2.0);


    iCub.home();
    yInfo("try to grasp %s", target.c_str());
    bool ok=iCub.grasp(target,options);
    yInfo()<<(ok?"grasped!":"missed!");
    iCub.home();

    if (ok)
    {
        x[2]+=1.2*radius;
        yInfo()<<"releasing ... ";
        iCub.release(x);
        iCub.home();
    }

    yInfo()<<"shutting down ... ";
    iCub.close();
    return 0;
}


