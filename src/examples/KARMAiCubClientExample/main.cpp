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
        yError()<<"[KARMAiCubClientExample] YARP network seems unavailable!";
        return -1;
    }

    ICubClient iCub("KARMAiCubClientExample","icubClient","example_ARE_KARMA.ini");

    // we connect to ARE and KARMA, which is defined in "example_ARE_KARMA.ini"
    if (!iCub.connectSubSystems())
    {
        yError()<<"[KARMAiCubClientExample] KARMA seems unavailabe!";
        return -1;
    }

    // object location in the iCub frame
    Vector x(4);
    x[0]=-0.30;
    x[1]=0.05;
    x[2]=-0.05;
    x[3]=1.0;

    // corresponding location in the world frame
    Matrix T(4,4);
    T(0,0)=0.0;  T(0,1)=-1.0; T(0,2)=0.0; T(0,3)=0.0;
    T(1,0)=0.0;  T(1,1)=0.0;  T(1,2)=1.0; T(1,3)=0.5976;
    T(2,0)=-1.0; T(2,1)=0.0;  T(2,2)=0.0; T(2,3)=-0.026;
    T(3,0)=0.0;  T(3,1)=0.0;  T(3,2)=0.0; T(3,3)=1.0;
    Vector wx=T*x;
    x.pop_back();

    // push left with pure API of KARMA. Please see KARMA document for details of how to define arguments
    iCub.home();                            // Home by using ARE
    x[1] =x[1] - 0.1;
    yInfo()<<"[KARMAiCubClientExample] try to push left with KARMA...";
    bool ok = iCub.pushKarma(x,180,0.2);    // Object will be push to left with y-coordinate as -0.05 from -0.25
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    // push right with pure API of KARMA. Please see KARMA document for details of how to define arguments
    iCub.home();                            // Home by using ARE
    yInfo()<<"[KARMAiCubClientExample] try to push right with KARMA...";
    ok = iCub.pushKarma(x,0,0.2);           // Object will be push to right with y-coordinate as -0.05 from 0.15
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    // push front with pure API of KARMA. Please see KARMA document for details of how to define arguments
    iCub.home();                            // Home by using ARE
    x[0] = x[0] - 0.15;
    yInfo()<<"[KARMAiCubClientExample] try to push front with KARMA...";
    ok = iCub.pushKarma(x,-90,0.2);         // Object will be push to front with x-coordinate as -0.45 from -0.25
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    // pull back with pure API of KARMA. Please see KARMA document for details of how to define arguments
    iCub.home();                            // Home by using ARE
    x[0] = -0.45;
    x[1] =  0.1;
    x[2] = -0.05;
    yInfo()<<"[KARMAiCubClientExample] try to pull with KARMA...";
    ok = iCub.drawKarma(x,0,0,0.2);         // Object will be pull back 0.2m with x-coordinate from -0.45
    yInfo()<<(ok?"success":"failed");
    Time::delay(4.0);

    yInfo()<<"[KARMAiCubClientExample] shutting down ... ";
    iCub.close();
    return 0;
}


