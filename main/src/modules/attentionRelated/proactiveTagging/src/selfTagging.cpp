/* 
 * Copyright (C) 2015 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Grégoire Pointeau, Tobias Fischer, Maxime Petit
 * email:   greg.pointeau@gmail.com, t.fischer@imperial.ac.uk, m.petit@imperial.ac.uk
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later versions published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * wysiwyd/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include "proactiveTagging.h"

using namespace yarp::os;
using namespace yarp::sig;
using namespace wysiwyd::wrdac;
using namespace std;

/*
* Explore an unknown tactile entity (e.g. fingertips), when knowing the name
* @param: Bottle with (exploreTactileUnknownEntity entityType entityName) (eg: exploreUnknownEntity agent unknown_25)
* @return Bottle with the result (error or ack?)
*/
yarp::os::Bottle proactiveTagging::exploreTactileEntityWithName(Bottle bInput) {  
    Bottle bOutput;

    if (bInput.size() != 3)
    {
        yInfo() << " proactiveTagging::exploreTactileEntityWithName | Problem in input size.";
        bOutput.addString("Problem in input size");
        iCub->say("Error in input size explore tactile with name");
        return bOutput;
    }

    string sBodyPart = bInput.get(1).toString();
    string sName = bInput.get(2).toString();

    yInfo() << " EntityType : " << sBodyPart;

    //1. search through opc for the bodypart entity
    iCub->opc->checkout();
    Bodypart* BPentity = dynamic_cast<Bodypart*>(iCub->opc->getEntity(sName, true));
    if(!BPentity) {
        iCub->say("Could not cast to bodypart in tactile");
        yError() << "Could not cast to bodypart in tactile";
        bOutput.addString("nack");
        return bOutput;
    }

    if (!Network::isConnected(touchDetectorRpc.c_str(), portFromTouchDetector.getName().c_str())) {
	    if (!Network::connect(touchDetectorRpc.c_str(), portFromTouchDetector.getName().c_str())) {
		yWarning() << "TOUCH DETECTOR NOT CONNECTED: selfTagging will not work";
	    }
    }

    //2.Ask human to touch
    string sAsking = "I know how to move my " + getBodyPartNameForSpeech(sName) + ", but how does it feel when I touch something? Can you touch my " + getBodyPartNameForSpeech(sName) + " when I move it, please?";
    yInfo() << " sAsking: " << sAsking;
    iCub->lookAtPartner();
    iCub->say(sAsking, false);
    portFromTouchDetector.read(false); // clear buffer from previous readings

    yarp::os::Time::delay(1.0);

    //2.b Move also the bodypart to show it has been learnt.
    yInfo() << "Cast okay : name BP = " << BPentity->name();
    int joint = BPentity->m_joint_number;
    //send rpc command to bodySchema to move the corresponding part
    yInfo() << "Start bodySchema";
    double babbling_duration = 3.0;
    iCub->babbling(joint, babblingArm, babbling_duration);

    //3. Read until some tactile value are detected
    //TODO: Here, instead we should check the saliency given by pasar!
    bool gotTouch = false;
    Bottle *bTactile;
    int timeout = 0;
    while(!gotTouch && timeout<10) {
        bTactile = portFromTouchDetector.read(false);
        if(bTactile != NULL) {
            gotTouch = true;
            break;
        } else {
            yarp::os::Time::delay(0.5);
            timeout++;
        }
    }

    if(!gotTouch) {
        yError() << " error in proactiveTagging::exploreTactileEntityWithName | for " << sName << " | Touch not detected!" ;
        bOutput.addString("error");
        bOutput.addString("I did not feel any touch.");
        iCub->say("I did not feel any touch.", false);
        iCub->home();

        return bOutput;
    }

    //4. Assign m_tactile_number
    BPentity->m_tactile_number = bTactile->get(0).asInt();
    bOutput.addString("ack");
    bOutput.addInt(bTactile->get(0).asInt());
    yDebug() << "Assigned" << BPentity->name() << "(id)" << BPentity->opc_id() << "tactile_number to" << bTactile->get(0).asInt();
    iCub->opc->commit(BPentity);

    //4.Ask human to touch
    string sThank = " Thank you, now I know when I am touching object with my " + getBodyPartNameForSpeech(sName);
    yInfo() << " sThank: " << sThank;
    iCub->lookAtPartner();
    iCub->say(sThank);
    iCub->home();

    return bOutput;
}
