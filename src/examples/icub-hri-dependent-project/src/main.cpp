/*
* Copyright(C) 2017 WYSIWYD Consortium, European Commission FP7 Project ICT - 612139
* Authors: Tobias Fischer
* email : t.fischer@imperial.ac.uk
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

#include <string>
#include <list>

#include <yarp/os/all.h>
#include <icubhri/all.h>

using namespace yarp::os;
using namespace icubhri;

/*******************************************************/
class ExampleDependency : public RFModule
{
    double period;
    RpcServer rpcPort;
    ICubClient icubclient;

public:
    /*******************************************************/
    ExampleDependency() : icubclient("exampleDependency", "exampleDependency") { }

    /*******************************************************/
    bool configure(ResourceFinder &rf)
    {
        period=rf.check("period",Value(0.05)).asDouble();

        if (!icubclient.connect()) {
            yError() << " iCubClient : Some dependencies are not running (OPC?)...";
            return false;
        }

        rpcPort.open("/exampleDependency/rpc");
        attach(rpcPort);

        return true;
    }

    /*******************************************************/
    double getPeriod()
    {
        return period;
    }

    /*******************************************************/
    bool updateModule()
    {
        return true;
    }

    /*******************************************************/
    bool respond(const Bottle &command, Bottle &reply)
    {
        int cmd=command.get(0).asVocab();
        int ack=Vocab::encode("ack");
        int nack=Vocab::encode("nack");

        if (cmd==Vocab::encode("lookAtPartner"))
        {
            icubclient.lookAtPartner();
            reply.addVocab(ack);
            return true;
        }
        else
            return RFModule::respond(command,reply);
    }

    /*******************************************************/
    bool close()
    {
        rpcPort.interrupt();
        rpcPort.close();
        icubclient.close();
        return true;
    }

    bool interruptModule()
    {
        rpcPort.interrupt();
        icubclient.opc->interrupt();
        return true;
    }
};


/*******************************************************/
int main(int argc, char *argv[])
{
    Network yarp;
    if (!yarp.checkNetwork()) {
        yError()<<"YARP network seems unavailable!";
        return 1;
    }

    ResourceFinder rf;
    rf.configure(argc,argv);

    ExampleDependency example;
    return example.runModule(rf);
}
