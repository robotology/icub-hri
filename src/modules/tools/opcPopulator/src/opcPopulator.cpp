#include <functional>
#include "opcPopulator.h"
#include "icubclient/clients/icubClient.h"
#include "icubclient/subsystems/subSystem_ARE.h"

using namespace std;
using namespace yarp::os;
using namespace icubclient;

bool opcPopulator::configure(yarp::os::ResourceFinder &rf)
{
    string moduleName = rf.check("name", Value("opcPopulator")).asString().c_str();
    setName(moduleName.c_str());

    yInfo() << moduleName << " : finding configuration files...";
    period = rf.check("period", Value(0.1)).asDouble();

    //Create an iCub Client and check that all dependencies are here before starting
    bool isRFVerbose = false;
    iCub = new ICubClient(moduleName, "opcPopulator", "client.ini", isRFVerbose);
    iCub->opc->isVerbose = false;
    if (!iCub->connect())
    {
        yInfo() << " iCubClient : Some dependencies are not running...";
        Time::delay(1.0);
    }
    rpc.open(("/" + moduleName + "/rpc").c_str());
    attach(rpc);

    Bottle &bRFInfo = rf.findGroup("populateSpecific");

    X_obj = bRFInfo.check("X_obj", Value(-0.4)).asDouble();
    Y_obj = bRFInfo.check("Y_obj", Value(0.5)).asDouble();
    Z_obj = bRFInfo.check("Z_obj", Value(0.1)).asDouble();
    X_ag = bRFInfo.check("X_ag", Value(-0.4)).asDouble();
    Y_ag = bRFInfo.check("Y_ag", Value(0.5)).asDouble();
    Z_ag = bRFInfo.check("Z_ag", Value(0.5)).asDouble();
    noise = bRFInfo.check("noise", Value(5.1)).asDouble();

    yInfo() << "X_obj " << X_obj;
    yInfo() << "Y_obj " << Y_obj;
    yInfo() << "Z_obj " << Z_obj;
    yInfo() << "X_ag " << X_ag;
    yInfo() << "Y_ag " << Y_ag;
    yInfo() << "Z_ag " << Z_ag;
    yInfo() << "noise " << noise;

    iCub->home();

    yInfo() << "\n \n" << "----------------------------------------------" << "\n \n" << moduleName << " ready ! \n \n ";


    return true;
}


bool opcPopulator::close() {
    iCub->close();
    delete iCub;

    return true;
}


bool opcPopulator::respond(const Bottle& command, Bottle& reply) {
    string helpMessage = string(getName().c_str()) +
        " commands are: \n" +
        "help \n" +
        "quit \n" +
        "populateEntityRandom entity_type entity_name \n" +
        "populateSpecific \n" +
        "populateTwoUnknowns \n" +
        "populateRedBall \n" +
        "addUnknownEntity entity_type\n" +
        "setSaliencyEntity entity_name saliency_value\n" +
        "setValueEntity entity_name value_value\n";

    reply.clear();


    if (command.get(0).asString() == "quit") {
        reply.addString("quitting");
        return false;
    }
    else if (command.get(0).asString() == "populateRedBall") {
        yInfo() << " populateRedBall";
        (populateRedBall()) ? reply.addString("populateRedBall done !") : reply.addString("populateRedBall failed !");
    }
    else if (command.get(0).asString() == "populateEntityRandom") {
        yInfo() << " populateEntityRandom";
        (populateEntityRandom(command)) ? reply.addString("populateSpecific done !") : reply.addString("populateSpecific failed !");
    }
    else if (command.get(0).asString() == "populateSpecific") {
        yInfo() << " populateSpecific";
        (populateSpecific()) ? reply.addString("populateSpecific done !") : reply.addString("populateSpecific failed !");
    }
    else if (command.get(0).asString() == "populateTwoUnknowns") {
        yInfo() << " populateTwoUnknowns";
        (populateTwoUnknowns()) ? reply.addString("populated 2 objects !") : reply.addString("populateSpecific failed !");
    }
    else if (command.get(0).asString() == "addUnknownEntity") {
        yInfo() << " addUnknownEntity";
        (addUnknownEntity(command)) ? reply.addString("addUnknownEntity done !") : reply.addString("addUnknownEntity failed !");
    }
    else if (command.get(0).asString() == "setSaliencyEntity") {
        yInfo() << " setSaliencyEntity";
        (setAttributeEntity(command, std::mem_fn(&Object::setSaliency))) ? reply.addString("setSaliencyEntity done !") : reply.addString("setSaliencyEntity failed !");
    }
    else if (command.get(0).asString() == "setValueEntity") {
        yInfo() << " setValueEntity";
        (setAttributeEntity(command, std::mem_fn(&Object::setValue))) ? reply.addString("setValueEntity done !") : reply.addString("setValueEntity failed !");
    }
    else if (command.get(0).asString() == "clear") {
        yInfo() << " clearing OPC";
        iCub->opc->clear();
        iCub->opc->update();
        reply.addString("clearing OPC");
    }
    else {
        yInfo() << helpMessage;
        reply.addString("wrong command");
        reply.addString(helpMessage);
    }

    return true;
}

/* Called periodically every getPeriod() seconds */
bool opcPopulator::updateModule() {
    return true;
}

bool opcPopulator::populateEntityRandom(const Bottle& bInput){

    if (bInput.size() != 3)
    {
        yWarning() << " in opcPopulator::populateEntityRandom | wrong number of input";
        return false;
    }
    string sName = bInput.get(2).toString();

    if (bInput.get(1).toString() == ICUBCLIENT_OPC_ENTITY_AGENT)
    {
        Agent* agent = iCub->opc->addOrRetrieveEntity<Agent>(sName);
        agent->m_ego_position[0] = (-1.5) * (Random::uniform()) - 0.5;
        agent->m_ego_position[1] = (2) * (Random::uniform()) - 1;
        agent->m_ego_position[2] = 0.60;
        agent->m_present = 1.0;
        agent->m_color[0] = Random::uniform(0, 80);
        agent->m_color[1] = Random::uniform(180, 250);
        agent->m_color[2] = Random::uniform(80, 180);
        iCub->opc->commit(agent);
    }
    else if (bInput.get(1).toString() == ICUBCLIENT_OPC_ENTITY_OBJECT)
    {
        Object* obj = iCub->opc->addOrRetrieveEntity<Object>(sName);
        obj->m_ego_position[0] = (-1.5) * (Random::uniform()) - 0.2;
        obj->m_ego_position[1] = (2) * (Random::uniform()) - 1;
        obj->m_ego_position[2] = 0.20;
        obj->m_present = 1.0;
        obj->m_color[0] = Random::uniform(100, 180);
        obj->m_color[1] = Random::uniform(0, 80);
        obj->m_color[2] = Random::uniform(180, 250);
        obj->m_value = 1.5;
        yDebug() << "value: " << obj->m_value;
        iCub->opc->commit(obj);
    }
    else
    {
        yError() << "Entity type not supported";
        return false;
    }

    return true;
}


bool opcPopulator::addUnknownEntity(const Bottle &bInput){

    if (bInput.size() != 2)
    {
        yWarning() << " in opcPopulator::addUnknownEntity | wrong number of input";
        return false;
    }

    iCub->opc->checkout();
    string sName = "unknown";
    yInfo() << " to be added: " << bInput.get(1).toString() << " called " << sName;

    if (bInput.get(1).toString() == ICUBCLIENT_OPC_ENTITY_AGENT)
    {
        sName = "partner";
        Agent* agent = iCub->opc->addEntity<Agent>(sName);
        agent->m_ego_position[0] = (-1.5) * (Random::uniform()) - 0.5;
        agent->m_ego_position[1] = (2) * (Random::uniform()) - 1;
        agent->m_ego_position[2] = 0.60;
        agent->m_present = 1.0;
        agent->m_color[0] = Random::uniform(0, 80);
        agent->m_color[1] = Random::uniform(180, 250);
        agent->m_color[2] = Random::uniform(80, 180);
        iCub->opc->commit(agent);
    }
    else if (bInput.get(1).toString() == ICUBCLIENT_OPC_ENTITY_OBJECT)
    {
        Object* obj = iCub->opc->addEntity<Object>(sName);
        obj->m_ego_position[0] = (-1.5) * (Random::uniform()) - 0.2;
        obj->m_ego_position[1] = (2) * (Random::uniform()) - 1;
        obj->m_ego_position[2] = 0.20;
        obj->m_present = 1.0;
        obj->m_color[0] = Random::uniform(100, 180);
        obj->m_color[1] = Random::uniform(0, 80);
        obj->m_color[2] = Random::uniform(180, 250);
        iCub->opc->commit(obj);
    }
    else
    {
        yError() << bInput.get(1).toString() << " unknown kind of entity";
        return false;
    }

    return true;
}

bool opcPopulator::setAttributeEntity(const Bottle& bInput, std::function<void(Object*, double)> f_setter){

    if (bInput.size() != 3) {
        yWarning() << " in opcPopulator::setAttributeEntity| wrong number of input";
        return false;
    }

    string sName = bInput.get(1).toString();
    double target = bInput.get(2).asDouble();

    iCub->opc->checkout();

    Entity *e = iCub->opc->getEntity(sName);
    if (e && (e->entity_type() == ICUBCLIENT_OPC_ENTITY_AGENT || e->entity_type() == ICUBCLIENT_OPC_ENTITY_OBJECT)) {
        f_setter(dynamic_cast<Object*>(e), target);
        iCub->opc->commit();
    } else{
        yError() << "Cannot change value of the non-object entity: " << sName;
        return false;
    }

    return true;
}


bool opcPopulator::populateSpecific() {
    iCub->opc->clear();

    double errorMargin = noise;

    Object* obj1 = iCub->opc->addOrRetrieveEntity<Object>("bottom_left");
    obj1->m_ego_position[0] = X_obj + errorMargin * (Random::uniform() - 0.5);
    obj1->m_ego_position[1] = -1.* Y_obj + errorMargin * (Random::uniform() - 0.5);
    obj1->m_ego_position[2] = Z_obj + errorMargin * (Random::uniform() - 0.5);
    obj1->m_present = 1.0;
    obj1->m_color[0] = Random::uniform(0, 80);
    obj1->m_color[1] = Random::uniform(80, 180);
    obj1->m_color[2] = Random::uniform(180, 250);
    iCub->opc->commit(obj1);

    Object* obj2 = iCub->opc->addOrRetrieveEntity<Object>("top_left");
    obj2->m_ego_position[0] = X_ag + errorMargin * (Random::uniform() - 0.5);
    obj2->m_ego_position[1] = -1.* Y_ag + errorMargin * (Random::uniform() - 0.5);
    obj2->m_ego_position[2] = Z_ag + errorMargin * (Random::uniform() - 0.5);
    obj2->m_present = 1.0;
    obj2->m_color[0] = Random::uniform(0, 180);
    obj2->m_color[1] = Random::uniform(0, 80);
    obj2->m_color[2] = Random::uniform(180, 250);
    iCub->opc->commit(obj2);

    Object* obj3 = iCub->opc->addOrRetrieveEntity<Object>("top_right");
    obj3->m_ego_position[0] = X_ag + errorMargin * (Random::uniform() - 0.5);
    obj3->m_ego_position[1] = Y_ag + errorMargin * (Random::uniform() - 0.5);
    obj3->m_ego_position[2] = Z_ag + errorMargin * (Random::uniform() - 0.5);
    obj3->m_present = 1.0;
    obj3->m_color[0] = Random::uniform(100, 180);
    obj3->m_color[1] = Random::uniform(80, 180);
    obj3->m_color[2] = Random::uniform(0, 80);
    iCub->opc->commit(obj3);

    Object* obj4 = iCub->opc->addOrRetrieveEntity<Object>("bottom_right");
    obj4->m_ego_position[0] = X_obj + errorMargin * (Random::uniform() - 0.5);
    obj4->m_ego_position[1] = Y_obj + errorMargin * (Random::uniform() - 0.5);
    obj4->m_ego_position[2] = Z_obj + errorMargin * (Random::uniform() - 0.5);
    obj4->m_present = 1.0;
    obj4->m_color[0] = Random::uniform(100, 180);
    obj4->m_color[1] = Random::uniform(0, 80);
    obj4->m_color[2] = Random::uniform(180, 250);
    iCub->opc->commit(obj4);

    return true;
}


bool opcPopulator::populateTwoUnknowns() {
    iCub->opc->clear();

    Object* obj1 = iCub->opc->addOrRetrieveEntity<Object>("unknown_1");
    obj1->m_ego_position[0] = -0.4;
    obj1->m_ego_position[1] = 0.25;
    obj1->m_ego_position[2] = 0;
    obj1->m_present = 1.0;
    obj1->m_color[0] = Random::uniform(0, 80);
    obj1->m_color[1] = Random::uniform(80, 180);
    obj1->m_color[2] = Random::uniform(180, 250);
    iCub->opc->commit(obj1);

    Object* obj2 = iCub->opc->addOrRetrieveEntity<Object>("unknown_2");
    obj2->m_ego_position[0] = -0.4;
    obj2->m_ego_position[1] = -0.25;
    obj2->m_ego_position[2] = 0;
    obj2->m_present = 1.0;
    obj2->m_color[0] = Random::uniform(100, 180);
    obj2->m_color[1] = Random::uniform(0, 80);
    obj2->m_color[2] = Random::uniform(180, 250);
    iCub->opc->commit(obj2);

    return true;
}


bool opcPopulator::populateRedBall(){

    Object* obj1 = iCub->opc->addOrRetrieveEntity<Object>("red_ball");
    obj1->m_ego_position[0] = -0.4;
    obj1->m_ego_position[1] = 0.25;
    obj1->m_ego_position[2] = 0;
    obj1->m_present = 0.0;
    obj1->m_color[0] = 250;
    obj1->m_color[1] = 0;
    obj1->m_color[2] = 0;
    obj1->m_value = -1.0;
    iCub->opc->commit(obj1);
    return true;
}
