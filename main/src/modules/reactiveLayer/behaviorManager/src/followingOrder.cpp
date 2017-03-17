#include "followingOrder.h"

using namespace std;
using namespace yarp::os;
using namespace wysiwyd::wrdac;

void FollowingOrder::configure() {
    Bottle bFollowingOrder = rf.findGroup("followingOrder");

    homeoPort = "/homeostasis/rpc";

    babblingArm = bFollowingOrder.find("babblingArm").asString();

    // Todo: set the value beow from a config file (but we are not in a module here)
    external_port_name = "/proactiveTagging/rpc";
    from_sensation_port_name = "/ears/target:o";

    port_to_homeo_name = "/"+behaviorName+"/toHomeo:o";
    port_to_homeo.open(port_to_homeo_name);

    manual = true;
}

void FollowingOrder::run(const Bottle &args) {
    yInfo() << "FollowingOrder::run";
    yDebug()<< args.toString();
    if (!Network::isConnected(port_to_homeo_name,homeoPort)){
        if (!Network::connect(port_to_homeo_name,homeoPort)){
            yWarning()<<"Port to Homeostasis not available. Could not freeze the drives...";
        }
    }

    if (Network::isConnected(port_to_homeo_name,homeoPort)){
        yInfo()<<"freezing drives";
        Bottle cmd;
        Bottle rply;
        cmd.addString("freeze");
        cmd.addString("all");

        port_to_homeo.write(cmd, rply);
    }

    Bottle* sens = args.get(0).asList();
    string action = sens->get(0).asString();
    string type;
    string target;
    if (sens->size()>0)
        type = sens->get(1).asString();
    if (sens->size()>1)
        target = sens->get(2).asString();

    yDebug() << "Action:" << action;
    yDebug() << "Type:" << type;
    yDebug() << "Target:" << target;

    if ( target != "none" && type != "bodypart" && type != "kinematic structure" && type != "kinematic structure correspondence"){           //we dont have searchEntity for bodypart
        bool verboseSearch=true;
        if( action == "this is" ) {
            verboseSearch=false;
        }
        yInfo() << "there are objects to search!!!";
        handleSearch(type, target, verboseSearch);
    }

    //FollowingOrder implying objects
    if ( (action == "point" || action == "look at" || action == "push" || action == "this is") && type == "object"){
        // Be careful: both handlePoint (point in response of a human order) and handlePointing (point what you know)
        if (sens->size()<2){
            iCub->say("I can't " + action + "if you don't tell me the object");
        } else {
            handleAction(type, target, action);
        }
    } else if (action == "move" && type == "bodypart") { //FollowingOrder implying bodypart
        if (sens->size()<2) {
            iCub->say("I can't " + action + "if you don't tell me the bodypart");
        } else {
            handleActionBP(type, target, action);
        }
    } else if (action == "end") {
        handleEnd();
    } else if (action == "game") {
        handleGame(type);
    } else {
        iCub->say("I don't know what you mean.");
    }

    if (!manual && Network::isConnected(port_to_homeo_name, homeoPort)){
        yInfo()<<"unfreezing drives";
        Bottle cmd;
        Bottle rply;
        cmd.addString("unfreeze");
        cmd.addString("all");
        port_to_homeo.write(cmd, rply);
    }
}

bool FollowingOrder::handleAction(string type, string target, string action) {
    yInfo() << "[handleAction] type: " << type << "target:" << target << "action:" << action;
    if(action == "this is") {
        yDebug() << "[handleAction] For action \"" + action + "\" there is nothing to do here. Return.";
        return true;
    }

    iCub->opc->checkout();
    yInfo() << " [handleAction]: opc checkout";
    list<Entity*> lEntities = iCub->opc->EntitiesCache();

    for (auto& entity : lEntities) {
        if (entity->name() == target) {
            if (entity->entity_type() == "object") {
                Object* o = dynamic_cast<Object*>(entity);
                if(o && o->m_present==1.0) {
                    yInfo() << "I'd like to" << action << "the" << target;

                    if(action == "point") {
                        iCub->say("oh! this is a " + target + ".", false);
                        iCub->point(target);
                    } else if(action == "look at") {
                        iCub->say("oh! look at the " + target + ".", false);
                        iCub->look(target);
                        yarp::os::Time::delay(1.5);
                    } else if(action == "push") {
                        iCub->say("oh! look how I pushed the " + target + ".", false);
                        iCub->push(target);
                    } else {
                        yError() << "This action is not supported!";
                    }
                    yarp::os::Time::delay(0.1);
                    iCub->home();

                    return true;
                } else if (o && o->m_present==0.0) {
                    iCub->lookAtPartner();
                    iCub->say("I know the " + target + " but it is not here.");
                    iCub->home();
                    return true;
                }
            }
        }
    }

    yWarning() << "Cannot" << action << "the" << target;
    iCub->lookAtPartner();
    iCub->say("I don't know the " + target + " but I know it is not here. I will not " + action + " it.");
    iCub->home();
    return false;
}

bool FollowingOrder::handleActionBP(string type, string target, string action) {
    yInfo() << "[handleActionBP] type: " << type << "target:" << target << "action:" << action;
    iCub->opc->checkout();
    yInfo() << " [handleActionBP]: opc checkout";
    list<Entity*> lEntities = iCub->opc->EntitiesCache();

    for (auto& entity : lEntities)
    {
        if (entity->name() == target) {
            if (entity->entity_type() == type) //type has been checked to be "bodypart" before, in run()
            {
                Bodypart* BPentity = dynamic_cast<Bodypart*>(entity);
                iCub->lookAtPartner();
                iCub->say("Nice, I will do some exercise with my " + babblingArm + " arm");

                if(action == "move") { //only action available right now for bodypart but keep the check for future development
                    int joint = BPentity->m_joint_number;
                    //send rpc command to bodySchema to move the corresponding part
                    yInfo() << "Start bodySchema";
                    double babbling_duration = 4.0;
                    iCub->say("I will move my " + target, false);
                    iCub->babbling(joint, babblingArm, babbling_duration);
                }

                iCub->home();

                return true;
            }
        }
    }

    yWarning() << "Cannot" << action << "my" << target;
    iCub->lookAtPartner();
    iCub->say("I cannot " + action + " my " + target + ", I do not know this word");
    iCub->home();
    return false;
}

bool FollowingOrder::handleSearch(string type, string target, bool verboseSearch)
{
    // look if the object (from human order) exist and if not, trigger proactivetagging

    iCub->opc->checkout();
    yInfo() << " [handleSearch] : opc checkout";
    list<Entity*> lEntities = iCub->opc->EntitiesCache();

    for (auto& entity: lEntities)
    {
        string sName = entity->name();
        if (sName == target) {
            yDebug() << "Entity found: "<<target;
            if (entity->entity_type() == "object")
            {
                Object* o = dynamic_cast<Object*>(iCub->opc->getEntity(sName));
                yInfo() << "I found the entity in the opc: " << sName;
                if(o && o->m_present==1.0) {
                    return true;
                }
            }
        }
    }

    yInfo() << "I need to explore by name!";

    // ask for the object
    yInfo() << "send rpc to proactiveTagging";

    //If there is an unknown object (to see with agents and objects), add it to the rpc_command bottle, and return true
    Bottle cmd;
    Bottle rply;

    cmd.addString("searchingEntity");
    cmd.addString(type);
    cmd.addString(target);
    cmd.addInt(verboseSearch);
    rpc_out_port.write(cmd,rply);
    yDebug() << rply.toString();

    return true;
}

bool FollowingOrder::handleEnd() {
    iCub->opc->checkout();
    yInfo() << "[handleEnd] time to sleep";
    iCub->lookAtPartner();
    iCub->say("Nice to play with you! See you soon.");
    iCub->home();

    yInfo()<<"[handleEnd] freezing drives";
    manual = true;
    Bottle cmd, rply;
    cmd.addString("freeze");
    cmd.addString("all");
    port_to_homeo.write(cmd, rply);

    return true;
}

bool FollowingOrder::handleGame(string type) {
    iCub->opc->checkout();
    yInfo() << "[handleGame] type:" << type;
    iCub->lookAtPartner();

    yInfo()<<"[handleGame] freezing drives";
    manual = true;
    Bottle homeo_cmd, homeo_reply;
    if(type == "start") {
        homeo_cmd.addString("freeze");
    } else if(type == "end") {
        homeo_cmd.addString("unfreeze");
    }
    homeo_cmd.addString("all");
    port_to_homeo.write(homeo_cmd, homeo_reply);

    return true;
}
