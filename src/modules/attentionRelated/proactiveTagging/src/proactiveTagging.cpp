/*
* Copyright (C) 2015 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
* Authors: Grégoire Pointeau, Tobias Fischer, Maxime Petit
* email:   greg.pointeau@gmail.com, t.fischer@imperial.ac.uk, m.petit@imperial.ac.uk
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

#include "proactiveTagging.h"

using namespace yarp::os;
using namespace yarp::sig;
using namespace icubclient;
using namespace std;

bool proactiveTagging::configure(yarp::os::ResourceFinder &rf) {
    string moduleName = rf.check("name", Value("proactiveTagging")).asString().c_str();
    setName(moduleName.c_str());

    GrammarAskNameObject = rf.findFileByName(rf.check("GrammarAskNameObject", Value("GrammarAskNameObject.xml")).toString());
    GrammarAskNameAgent = rf.findFileByName(rf.check("GrammarAskNameAgent", Value("GrammarAskNameAgent.xml")).toString());
    GrammarAskNameBodypart = rf.findFileByName(rf.check("GrammarAskNameBodypart", Value("GrammarAskNameSelf.xml")).toString());

    babblingArm = rf.check("babblingArm", Value("left")).toString();

    thresholdDistinguishObjectsRatio = rf.check("thresholdDistinguishObjectsRatio", Value(3.0)).asDouble();
    thresholdSalienceDetection = rf.check("thresholdSalienceDetection", Value(1.5)).asDouble();

    yDebug() << "------------> babblingArm: " << babblingArm;

    yInfo() << moduleName << ": finding configuration files...";
    period = rf.check("period", Value(0.1)).asDouble();

    defaultPartnerName = "partner";

    //Create an iCub Client and check that all dependencies are here before starting
    bool isRFVerbose = true;
    iCub = new ICubClient(moduleName, "proactiveTagging", "client.ini", isRFVerbose);
    iCub->opc->isVerbose &= true;

    while (!iCub->connect()) {
        yWarning() << "iCubClient : Some dependencies are not running...";
        Time::delay(1.0);
    }
    iCub->opc->checkout();

    configureOPC(rf);

    //--------------------------------------------- output port
    std::string ttsOptions = rf.check("ttsOptions", yarp::os::Value("iCub")).toString();
    if (ttsOptions != "iCub") {
        if (iCub->getSpeechClient())
            iCub->getSpeechClient()->SetOptions(ttsOptions);
    }

    //out to SAM
    portToSAM.open(("/" + moduleName + "/toSAM:o").c_str());
    SAMRpc = rf.check("SAMRpc", Value("/sam/rpc:i")).asString().c_str();

    if (!Network::connect(portToSAM.getName().c_str(), SAMRpc.c_str())) {
        yDebug() << "SAM NOT CONNECTED: face recognition will not work";
        iCub->say("SAM NOT CONNECTED");
    }

    // out to pasar
    portToPasar.open(("/" + moduleName + "/pasar:o").c_str());
    if (!Network::connect(portToPasar.getName().c_str(), "/pasar/rpc")) {
        yWarning() << "PASAR NOT CONNECTED: will not engage pointing";
        iCub->say("PASAR NOT CONNECTED");
    }

    //in from TouchDetector
    portFromTouchDetector.open(("/" + moduleName + "/fromTouch:i").c_str());
    touchDetectorRpc = rf.check("touchDetectorOut", Value("/touchDetector/touchClean:o")).asString().c_str();

    if (!Network::connect(touchDetectorRpc.c_str(), portFromTouchDetector.getName().c_str())) {
        yWarning() << "TOUCH DETECTOR NOT CONNECTED: selfTagging will not work";
        iCub->say("TOUCH DETECTOR NOT CONNECTED");
    }

    if (!iCub->getRecogClient()) {
        iCub->say("speech recognizer not connected");
        yWarning() << "SPEECH RECOGNIZER NOT CONNECTED";
    }

    //rpc port
    rpcPort.open(("/" + moduleName + "/rpc").c_str());
    attach(rpcPort);

    iCub->say("pro active tagging is ready", false);
    yInfo() << "\n \n" << "----------------------------------------------" << "\n \n" << moduleName << " ready ! \n \n ";

    iCub->home();

    return true;
}


bool proactiveTagging::interruptModule() {
    portFromTouchDetector.interrupt();
    portToSAM.interrupt();
    portToPasar.interrupt();
    rpcPort.interrupt();

    return true;
}

bool proactiveTagging::close() {
    if(iCub) {
        iCub->close();
        delete iCub;
    }

    portFromTouchDetector.interrupt();
    portFromTouchDetector.close();

    portToSAM.interrupt();
    portToSAM.close();

    portToPasar.interrupt();
    portToPasar.close();

    rpcPort.interrupt();
    rpcPort.close();

    return true;
}


bool proactiveTagging::respond(const Bottle& command, Bottle& reply) {
    string helpMessage = string(getName().c_str()) +
        " commands are: \n" +
        "help \n" +
        "quit \n" +
        "change_name oldname newname \n" +
        "exploreUnknownEntity entity_type entity_name \n" +
        "searchingEntity entity_type entity_name \n" +
        "exploreKinematicByName entity_name bodypart [true/false] \n" +
        "exploreKinematicByJoint joint bodypart [true/false] \n";

    reply.clear();


    if (command.get(0).asString() == "quit") {
        reply.addString("quitting");

        rpcPort.reply(reply);
        return false;
    }
    else if(command.get(0).asString() == "change_name") {
        string oldname = command.get(1).toString();
        string newname = command.get(2).toString();
        yInfo() << "change_name from" << oldname << "to" << newname;
        Entity *e = iCub->opc->getEntity(oldname);
        if(!e) {
            iCub->say("No entity with name " + oldname);
            yError() << "No entity with name " << oldname;
            reply.addString("nack");
        } else {
            iCub->changeName(e, newname);
            reply.addString("ack");
        }
    }
    else if (command.get(0).asString() == "exploreUnknownEntity") {
        // disable EARS early on
        iCub->getRecogClient()->listen(false);

        yInfo() << " exploreUnknownEntity";
        string type = command.get(1).toString();
        string name = command.get(2).toString();
        yDebug() << "exploreUnknownEntity with name = " << name;

        if ((type == "bodypart") && (name.find("unknown") == std::string::npos)) { //type is bodypart and it already has a name
            iCub->opc->checkout();
            Bodypart* bp = dynamic_cast<Bodypart*>(iCub->opc->getEntity(name));
            if(!bp) {
                yError() << "Could not cast" << name << "to bodypart";
                iCub->say("Could not cast " + name + " to bodypart");
            } else {
                if (bp->m_tactile_number == -1) {
                    yInfo() << "Going to tag skin part";
                    reply = exploreTactileEntityWithName(command);
                } else {
                    yWarning("Not sure what to do, name and tactile information already known");
                }
            }
        } else if (type == "bodypart") {
            yInfo() << "Going to tag bodypart (include babbling)";
            reply = exploreUnknownEntity(command);
        }
        else if (type == "object") {
            yInfo() << "Going to tag object (include a pointing)";
            reply = exploreUnknownEntity(command);
        } else if (type == "agent") {
            yInfo() << "Going to tag an agent (face recog)";
            reply = exploreUnknownEntity(command);
        } else {
            yWarning() << "Type = " << type << " is NON valid for exploreUnknownEntity -> doing nothing";
            reply.addString("error");
            reply.addString("Type is NON valid for exploreUnknownEntity");
        }

        // enable EARS again
        iCub->getRecogClient()->listen(true);
    }
    else if (command.get(0).asString() == "searchingEntity") {
        yInfo() << " searchingEntity";
        reply = searchingEntity(command);
    }
    else {
        yInfo() << helpMessage;
        reply.addString(helpMessage);
    }

    rpcPort.reply(reply);

    return true;
}

/* Called periodically every getPeriod() seconds */
bool proactiveTagging::updateModule() {
    return true;
}

/*
* Recognize the name of an unknown entity
* Recognize through speech the name of an unknown entity.
* @return (error errorDescription) or (sName)
*/
Bottle proactiveTagging::recogName(string entityType) {
    Bottle bOutput;

    Bottle bRecognized, //received from speech recog with transfer information (1/0 (bAnswer))
           bAnswer;     //response from speech recog without transfer information, including raw sentence

    yDebug() << "Going to load grammar.";
    string grammar, expectedresponse="error", semanticfield="error";
    if (entityType == "agent") {
        grammar = GrammarAskNameAgent;
        expectedresponse = "SENTENCEAGENT";
        semanticfield = "agent";
    } else if (entityType == "object") {
        grammar = GrammarAskNameObject;
        expectedresponse = "SENTENCEOBJECT";
        semanticfield = "object";
    } else if (entityType == "bodypart") {
        grammar = GrammarAskNameBodypart;
        expectedresponse = "SENTENCEBODYPART";
        semanticfield = "fingerName";
    } else {
        yError() << " error in proactiveTagging::recogName | for " << entityType << " | Entity Type not managed";
        bOutput.addString("error");
        bOutput.addString("Entity Type not managed");
        iCub->say("I do not know what you want from me. Can you please ask me something else?");
        return bOutput;
    }

    // Load the Speech Recognition with grammar according to entityType
    // bAnswer is the result of the recognition system (first element is the raw sentence, second is the list of semantic element)
    bool recognizedCorrectGrammar=false;
    while(!recognizedCorrectGrammar) {
        bRecognized = iCub->getRecogClient()->recogFromGrammarLoop(grammarToString(grammar), 20);
        bAnswer = *bRecognized.get(1).asList();
        if(bAnswer.get(1).asList()->get(0).toString() != expectedresponse) {
            iCub->say("I did not understand you. Can you please repeat?");
            yError() << "Wrong sentence type returned (not " << expectedresponse << ")";
        } else {
            recognizedCorrectGrammar = true;
        }
    }

    yDebug() << "Response from recogClient: " << bRecognized.toString();

    if (bRecognized.get(0).asInt() == 0)
    {
        yError() << " error in proactiveTagging::askName | for " << entityType << " | Error in speechRecog";
        bOutput.addString("error");
        bOutput.addString("error in speechRecog");
        return bOutput;
    }

    string sSentence = bAnswer.get(0).asString();
    iCub->say("I've understood " + sSentence);

    if (bAnswer.get(0).asString() == "stop")
    {
        yError() << " in proactiveTagging::askName | for " << entityType << " | stop called";
        bOutput.addString("error");
        bOutput.addString("stop called");
        return bOutput;
    }

    Bottle bSemantic = *bAnswer.get(1).asList(); // semantic information of the content of the recognition
    string sName = bSemantic.check(semanticfield, Value("error")).asString();
    bOutput.addString(sName);
    return bOutput;
}

Bottle proactiveTagging::getNameFromSAM(string sNameTarget, string currentEntityType) {
    iCub->say("Have we met before?", false);
    Bottle bOutput;
    Bottle bToSam, bReplySam;
    bToSam.addString("ask_face_label");

    yDebug() << "Request to SAM: " << bToSam.toString();
    portToSAM.write(bToSam, bReplySam);
    yDebug() << "Reply from SAM: " << bReplySam.toString();
    string sNameSAM = bReplySam.get(0).asString();
    if(sNameSAM != "nack" && sNameSAM != "unknown" && sNameSAM != "" && sNameSAM != "None") {
        Agent* TARGET = dynamic_cast<Agent*>(iCub->opc->getEntity(sNameTarget));
        yDebug() << "Changing name from " << TARGET->name() << " to " << sNameSAM;
        iCub->changeName(TARGET,sNameSAM);
        iCub->opc->commit(TARGET);

        iCub->say("Yes, I remember. Nice to see you, " + sNameSAM);
        yarp::os::Time::delay(0.2);
        iCub->home();

        bOutput.addString("success");
        bOutput.addString(currentEntityType);
    }
    else {
        if(sNameSAM == "nack") {
             yError() << bReplySam.get(1).asString();
             //iCub->say("I could not get the name from SAM");
        }
        
        bOutput.addString("nack");
    }
    return bOutput;
}


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
    //send rpc command to babbling to move the corresponding part
    yInfo() << "Start babbling";
    double babbling_duration = 3.0;
    iCub->babbling(joint, babblingArm, babbling_duration);

    //3. Read until some tactile value are detected
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
    iCub->lookAtPartner();
    iCub->say(sThank);
    iCub->home();

    return bOutput;
}


/*
* Explore an unknown entity by asking the name
* input: exploreUnknownEntity entityType entityName (eg: exploreUnknownEntity agent unknown_25)
* ask through speech the name of an unknwon entity
*/
Bottle proactiveTagging::exploreUnknownEntity(const Bottle& bInput) {
    Bottle bOutput;
    Bottle tryRecog;
    if (bInput.size() != 3)
    {
        yInfo() << " proactiveTagging::exploreEntity | Problem in input size.";
        bOutput.addString("Problem in input size");
        return bOutput;
    }

    iCub->opc->checkout();

    string currentEntityType = bInput.get(1).toString();
    string sNameTarget = bInput.get(2).toString();

    yInfo() << " EntityType : " << currentEntityType;

    //Check if name is known or not. if yes, and body part : ask tactile

    //Ask question for the human, or ask to pay attention (if action to focus attention after)
    string sQuestion;
    if (currentEntityType == "agent") {
        iCub->lookAtPartner();

        if (!Network::connect(portToSAM.getName().c_str(), SAMRpc.c_str())) {
            yWarning() << " SAM NOT CONNECTED: face recognition will not work";
        }
        if(portToSAM.getOutputCount()>0) {
           tryRecog = getNameFromSAM(sNameTarget, currentEntityType);

           if (tryRecog.get(0).toString() == "nack" ){
              sQuestion = " No, I don't know you. What is your name?";
           }
           else{
              return tryRecog;
           }
        }
        else
        {
            sQuestion = " Hello, I don't know you. Who are you?";
        }
    }
    else if (currentEntityType == "object") {
        iCub->look(sNameTarget);
        sQuestion = " Hum, what is this object?";
    }
    else if (currentEntityType == "bodypart") {
        iCub->lookAtPartner();
        sQuestion = " Watch please, I will move a part of my body";
    }
    else {
        yError() << " error in proactiveTagging::exploreUnknownEntity | for " << currentEntityType << " | Entity Type not managed";
        bOutput.addString("nack");
        bOutput.addString("Entity Type not managed");
        return bOutput;
    }

    yInfo() << sQuestion;
    iCub->say(sQuestion, false);

    //Act to determine the entity to be named, according to entityType (e.g. bodypart is sending a command to move the joint, ...)
    if (currentEntityType == "bodypart") {
        Bodypart* BPtemp = dynamic_cast<Bodypart*>(iCub->opc->getEntity(sNameTarget));
        if(!BPtemp) {
            bOutput.addString("nack");
            bOutput.addString("Could not cast to Bodypart");
            iCub->say(bOutput.toString());
            return bOutput;
        }
        yInfo() << "Cast okay : name BP = " << BPtemp->name();
        int joint = BPtemp->m_joint_number;
        //send rpc command to babbling to move the corresponding part
        yInfo() << "Start babbling";
        double babbling_duration=3.0;
        iCub->babbling(joint, babblingArm, babbling_duration);

        //add name of the partner in the question if defined
        string partnerName = iCub->getPartnerName();
        iCub->lookAtPartner();
        if(partnerName == defaultPartnerName){
            sQuestion = " How do you call this part of my body?";
        } else {
            sQuestion = partnerName + ", How do you call this part of my body?";
        }

        yInfo() << sQuestion;
        iCub->say(sQuestion, false);
    }
    else if (currentEntityType == "object") {
        yDebug() << "Going to point " << sNameTarget;
        iCub->point(sNameTarget);
    }

    Bottle bName = recogName(currentEntityType);
    string sName;

    if (bName.get(0).asString() == "error") {
        return bName; //if error, bName = (error errorDescription) -> return it
    }
    else {
        sName = bName.get(0).asString();
    }

    string sReply;
    Entity* e = iCub->opc->getEntity(sNameTarget);
    iCub->changeName(e, sName);

    iCub->lookAtPartner();
    if (currentEntityType == "agent") {
        sReply = " Nice to meet you " + sName;
    }
    else if (currentEntityType == "object") {
        sReply = " I get it, this is a " + sName;
    }
    else if (currentEntityType == "bodypart") {
        sReply = " Nice, I know that I have a " + getBodyPartNameForSpeech(sName);
    } else {
        iCub->say("I do not know this entity type");
    }

    yInfo() << sReply;
    iCub->say(sReply, false);
    yarp::os::Time::delay(1.5);

    iCub->home();

    bOutput.addString("success");
    bOutput.addString(currentEntityType);

    yInfo() << "End of exploreUnknownEntity";

    return bOutput;
}

/*
* Search for the entity corresponding to a certain name in all the unknown entities
* return a bottle of 2 elements.
* First element is: error - warning - success
* Second element is: information about the action
*/
Bottle proactiveTagging::searchingEntity(const Bottle &bInput) {
    Bottle bOutput;

    if (bInput.size() < 3) {
        yInfo() << " proactiveTagging::searchingEntity | Problem in input size.";
        bOutput.addString("error");
        bOutput.addString("Problem in input size");
        return bOutput;
    }

    string sTypeTarget = bInput.get(1).toString();
    string sNameTarget = bInput.get(2).toString();
    bool verboseSearch = true;
    if(bInput.size() == 4) {
        verboseSearch = bInput.get(3).asInt() > 0;
    }
    yInfo() << " Entity to find: " << sNameTarget << "(Type: " << sTypeTarget << ", verbosity: " << verboseSearch << ")";

    int unknownEntitiesPresent = 0;

    // check if the entity is already present in the OPC
    if (iCub->opc->isConnected()) {
        iCub->opc->checkout();
        list<shared_ptr<Entity>> lEntities = iCub->opc->EntitiesCacheCopy();

        for (auto& entity : lEntities) {
            if (entity->name() == sNameTarget && entity->entity_type() == sTypeTarget) {
                yInfo() << " Entity " << sNameTarget << " is already known.";
                bOutput.addString("warning");
                bOutput.addString("entity already exists");
                return bOutput;
            }
            Object *o = dynamic_cast<Object*>(entity.get());
            if(o && o->name().find("unknown") != string::npos && o->entity_type() == sTypeTarget && o->m_present == 1.0) {
                unknownEntitiesPresent++;
            }
        }

        if(unknownEntitiesPresent == 0) {
            yInfo() << "No unknown entity is present.";
            bOutput.addString("nack");
            bOutput.addString("No unknown entity is present.");
            return bOutput;
        } else if(unknownEntitiesPresent == 1) {
            yInfo() << "There is only one unknown entity";
            for (auto& entity : lEntities) {
                Object *o = dynamic_cast<Object*>(entity.get());
                if(o && o->name().find("unknown") != string::npos && o->entity_type() == sTypeTarget && o->m_present == 1.0) {
                    iCub->lookAtPartner();
                    iCub->say("There was only one object which I didn't know");
                    iCub->changeName(o, sNameTarget);
                    yDebug() << "Changed name for" << sNameTarget;
                    iCub->say("Now I know the " + sNameTarget, false);
                    iCub->home();

                    bOutput.addString("name changed");
                    return bOutput;
                }
            }
        }
    } else {
        yWarning() << " in proactiveTagging::searchingEntity | OPC not Connected";
        bOutput.addString("error");
        bOutput.addString("OPC not connected");

        return bOutput;
    }

    // if there is several objects unknown (or at least one)
    string sSentence;
    if(verboseSearch) {
        if(sTypeTarget == "object") {
            sSentence = "I don't known which of these objects is a " + sNameTarget + ". Can you show me the " + sNameTarget;
        } else if (sTypeTarget == "bodypart") {
            sSentence = "I don't known my " + sNameTarget + ". Can you please touch my " + sNameTarget;
        }

        //add name of the partner at the end of the question if defined
        string partnerName = iCub->getPartnerName();
        if(partnerName != defaultPartnerName){
            sSentence += ", " + partnerName;
        }

        iCub->lookAtPartner();
        iCub->say(sSentence);
        yInfo() << sSentence;
    }

    // activate pointing detection in pasar
    if(sTypeTarget == "object") {
        iCub->home();

        bool success = setPasarPointing(true);
        if(!success) {
            yError() << "Problem with pasar when setPasarPointing(true)";
            bOutput.addString("error");
            bOutput.addString("Problem with pasar");
            return bOutput;
        }
    }

    string sNameBestEntity = getBestEntity(sTypeTarget);
    if(sNameBestEntity=="none") {
        iCub->say("you did not point at any object");
        yError() << "you did not point at any object";
        bOutput.addString("error");
        bOutput.addString("no pointing received");
        return bOutput;
    }

    // change name
    Object* TARGET = dynamic_cast<Object*>(iCub->opc->getEntity(sNameBestEntity));
    if(sTypeTarget == "object") {
        iCub->look(TARGET->name());
    }

    iCub->changeName(TARGET,sNameTarget);

    yInfo() << " name changed: " << sNameBestEntity << " is now " << sNameTarget;
    bOutput.addString("name changed");
    iCub->say("Now I know the " + sNameTarget, false);

    // de-activate pointing detection in pasar
    if(sTypeTarget == "object") {
        bool success = setPasarPointing(false);
        if(!success) {
            yError() << "Problem with pasar when setPasarPointing(false)";
            bOutput.addString("error");
            bOutput.addString("Problem with pasar");
            return bOutput;
        }
    }

    iCub->home();

    return bOutput;
}
