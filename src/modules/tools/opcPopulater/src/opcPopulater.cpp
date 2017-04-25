#include <functional>
#include "opcPopulater.h"
#include "icubclient/subsystems/subSystem_ARE.h"

using namespace std;
using namespace yarp::os;
using namespace icubclient;

bool opcPopulater::configure(yarp::os::ResourceFinder &rf)
{
    string moduleName = rf.check("name", Value("opcPopulater")).asString().c_str();
    setName(moduleName.c_str());

    yInfo() << moduleName << " : finding configuration files...";
    period = rf.check("period", Value(0.1)).asDouble();

    //Create an iCub Client and check that all dependencies are here before starting
    bool isRFVerbose = false;
    iCub = new ICubClient(moduleName, "opcPopulater", "client.ini", isRFVerbose);
    iCub->opc->isVerbose = false;
    if (!iCub->connect())
    {
        yInfo() << " iCubClient : Some dependencies are not running...";
        Time::delay(1.0);
    }
    rpc.open(("/" + moduleName + "/rpc").c_str());
    attach(rpc);

    Bottle &bRFInfo = rf.findGroup("populateSpecific2");

    X_obj = bRFInfo.check("X_obj", Value(-0.4)).asDouble();
    Y_obj = bRFInfo.check("Y_obj", Value(0.4)).asDouble();
    Z_obj = bRFInfo.check("Z_obj", Value(0.0)).asDouble();
    X_ag = bRFInfo.check("X_ag", Value(-0.4)).asDouble();
    Y_ag = bRFInfo.check("Y_ag", Value(0.4)).asDouble();
    Z_ag = bRFInfo.check("Z_ag", Value(0.3)).asDouble();
    noise = bRFInfo.check("noise", Value(0.0)).asDouble();

    cout << "X_obj " << X_obj << endl;
    cout << "Y_obj " << Y_obj << endl;
    cout << "Z_obj " << Z_obj << endl;
    cout << "X_ag " << X_ag << endl;
    cout << "Y_ag " << Y_ag << endl;
    cout << "Z_ag " << Z_ag << endl;
    cout << "noise " << noise << endl;

    move = false;
    spd1.push_back(0.);
    spd1.push_back(0.);
    spd1.push_back(0.);
    spd2.push_back(0.);
    spd2.push_back(0.);
    spd2.push_back(0.);
    iter = 0;

    iCub->home();

    yInfo() << "\n \n" << "----------------------------------------------" << "\n \n" << moduleName << " ready ! \n \n ";


    return true;
}


bool opcPopulater::close() {
    iCub->close();
    delete iCub;

    return true;
}


bool opcPopulater::respond(const Bottle& command, Bottle& reply) {
    string helpMessage = string(getName().c_str()) +
        " commands are: \n" +
        "help \n" +
        "quit \n" +
        "populateSpecific1 entity_type entity_name \n" +
        "populateSpecific2 \n" +
        "populateSpecific3 \n" +
        "populateRedBall \n" +
        "populateMoving \n" +
        "addUnknownEntity entity_type\n" +
        "setSaliencyEntity entity_name saliency_name\n";

    reply.clear();


    if (command.get(0).asString() == "quit") {
        reply.addString("quitting");
        return false;
    }
    else if (command.get(0).asString() == "populateMoving") {
        yInfo() << " populateMoving";
        (populateMoving()) ? reply.addString("populateMoving done !") : reply.addString("populateMoving failed !");
    }
    else if (command.get(0).asString() == "populateRedBall") {
        yInfo() << " populateRedBall";
        (populateRedBall()) ? reply.addString("populateRedBall done !") : reply.addString("populateRedBall failed !");
    }
    else if (command.get(0).asString() == "populateSpecific1") {
        yInfo() << " populateSpecific1";
        (populateEntityRandom(command)) ? reply.addString("populateSpecific done !") : reply.addString("populateSpecific failed !");
    }
    else if (command.get(0).asString() == "populateSpecific2") {
        yInfo() << " populateSpecific2";
        (populateSpecific()) ? reply.addString("populateSpecific done !") : reply.addString("populateSpecific failed !");
    }
    else if (command.get(0).asString() == "populateSpecific3") {
        yInfo() << " populateSpecific3";
        (populateSpecific3()) ? reply.addString("populated 2 objects !") : reply.addString("populateSpecific failed !");
    }
    else if (command.get(0).asString() == "addUnknownEntity") {
        yInfo() << " addUnknownEntity";
        (addUnknownEntity(command)) ? reply.addString("addUnknownEntity done !") : reply.addString("addUnknownEntity failed !");
    }
    else if (command.get(0).asString() == "setSaliencyEntity") {
        yInfo() << " setSaliencyEntity";
        (setAttributeEntity(command, &Object::setSaliency)) ? reply.addString("setSaliencyEntity done !") : reply.addString("setSaliencyEntity failed !");
    }
    else if (command.get(0).asString() == "setValueEntity") {
        yInfo() << " setValueEntity";
        (setAttributeEntity(command, &Object::setValue)) ? reply.addString("setValueEntity done !") : reply.addString("setValueEntity failed !");
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
bool opcPopulater::updateModule() {
    //Just for testing: 
    if (move == true)
    {
        //double speed = 1./10.;
        iCub->opc->checkout();

        Object* obj1 = iCub->opc->addOrRetrieveEntity<Object>("moving_1");
        Object* obj2 = iCub->opc->addOrRetrieveEntity<Object>("moving_2");

        if (iter % 30 == 0)
        {
            spd1[0] = (rand() % 100)*pow(-1., (rand() % 2) + 1);
            spd1[1] = (rand() % 100)*pow(-1., (rand() % 2) + 1);
            spd1[2] = (rand() % 100)*pow(-1., (rand() % 2) + 1);
            spd2[0] = (rand() % 100)*pow(-1., (rand() % 2) + 1);
            spd2[1] = (rand() % 100)*pow(-1., (rand() % 2) + 1);
            spd2[2] = (rand() % 100)*pow(-1., (rand() % 2) + 1);
            double av1 = sqrt(pow(spd1[0], 2) + pow(spd1[1], 2) + pow(spd1[2], 2));
            double av2 = sqrt(pow(spd2[0], 2) + pow(spd2[1], 2) + pow(spd2[2], 2));
            spd1[0] /= av1;
            spd1[1] /= av1;
            spd1[2] /= av1;
            spd2[0] /= av2;
            spd2[1] /= av2;
            spd2[2] /= av2;
        }
        /* obj1->m_ego_position[0] = min(max(obj1->m_ego_position[0]+spd1[0]*period*speed,-0.2),-0.5);
         obj1->m_ego_position[1] = min(max(obj1->m_ego_position[1]+spd1[1]*period*speed,0.0),0.5);
         obj1->m_ego_position[2] = min(max(obj1->m_ego_position[2]+spd1[2]*period*speed,0.0),0.5);
         obj2->m_ego_position[0] = min(max(obj2->m_ego_position[0]+spd2[0]*period*speed,-0.2),-0.5);
         obj2->m_ego_position[1] = min(max(obj2->m_ego_position[1]+spd2[1]*period*speed,0.0),0.5);
         obj2->m_ego_position[2] = min(max(obj2->m_ego_position[2]+spd2[2]*period*speed,0.0),0.5);*/
        obj1->m_ego_position[0] = -0.18;
        obj1->m_ego_position[1] = 0.10;
        obj1->m_ego_position[2] = 0.1;
        obj2->m_ego_position[0] = -0.18;
        obj2->m_ego_position[1] = -0.10;
        obj2->m_ego_position[2] = 0.1;

        iCub->opc->commit(obj1);
        iCub->opc->commit(obj2);

        iter += 1;
    }
    return true;
}

bool opcPopulater::populateEntityRandom(const Bottle& bInput){

    if (bInput.size() != 3)
    {
        yWarning() << " in opcPopulater::populateSpecific | wrong number of input";
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

        agent = nullptr;
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

        obj = nullptr;
    }

    return true;
}


bool opcPopulater::addUnknownEntity(const Bottle &bInput){

    if (bInput.size() != 2)
    {
        yWarning() << " in opcPopulater::addUnknownEntity | wrong number of input";
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

        agent = nullptr;
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

        obj = nullptr;
    }
    else
    {
        yInfo() << " " << bInput.get(1).toString() << " unknown kind of entity";
        return false;
    }
    return true;
}

bool opcPopulater::setAttributeEntity(const Bottle& bInput, std::function<void(Object*, double)> f_setter){

    if (bInput.size() != 3) {
        yWarning() << " in opcPopulater::setAttributeEntity| wrong number of input";
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
        yWarning() << "Trying to change value of the non-object entity: " << sName << ". Please check!";
        return false;
    }

    return true;
}


bool opcPopulater::populateSpecific(){

    double errorMargin = noise;
    iCub->opc->clear();

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


bool opcPopulater::populateSpecific3(){

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


bool opcPopulater::populateMoving(){

    iCub->opc->clear();

    Object* obj1 = iCub->opc->addOrRetrieveEntity<Object>("moving_1");
    obj1->m_ego_position[0] = -0.18;
    obj1->m_ego_position[1] = 0.10;
    obj1->m_ego_position[2] = 0.1;

    obj1->m_present = 1.0;
    obj1->m_color[0] = 20;
    obj1->m_color[1] = 20;
    obj1->m_color[2] = 20;
    obj1->m_value = 0.0;
    iCub->opc->commit(obj1);

    Object* obj2 = iCub->opc->addOrRetrieveEntity<Object>("moving_2");
    obj2->m_ego_position[0] = -0.18;
    obj2->m_ego_position[1] = -0.10;
    obj2->m_ego_position[2] = 0.1;

    obj2->m_present = 1.0;
    obj2->m_color[0] = 200;
    obj2->m_color[1] = 20;
    obj2->m_color[2] = 20;
    obj2->m_value = -1.0;
    iCub->opc->commit(obj2);

    //move=true;


    return true;
}

bool opcPopulater::populateRedBall(){

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
