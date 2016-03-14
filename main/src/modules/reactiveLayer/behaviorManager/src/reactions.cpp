#include "reactions.h"

void Reactions::configure() {
    // Todo: set the value beow from a config file (but we are not in a module here)
    name = "reactions";
    external_port_name = "None";
    from_sensation_port_name = "None";
}

void Reactions::run(Bottle args/*=Bottle()*/) {
    yInfo() << "Reactions::run";

    Object* touchLocation = dynamic_cast<Object*>(iCub->opc->getEntity("touchLocation"));
    iCub->opc->update(touchLocation);
    touchLocation->m_present = true;
    iCub->opc->commit(touchLocation);

    iCub->say("Wow");
    iCub->look("touchLocation");//r.complement_place().c_str());
    iCub->say("Haaaahaha ... hahaaa");
    Time::delay(1.0);    

    yInfo() << "Reactions::run ends";

}