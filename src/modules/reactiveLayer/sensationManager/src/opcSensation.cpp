#include "opcSensation.h"
#include "icubclient/clients/icubClient.h"
#include "icubclient/clients/opcClient.h"

using namespace icubclient;
using namespace yarp::os;
using namespace std;

void OpcSensation::configure()
{
    bool isRFVerbose = false;
    iCub = new ICubClient("opcSensation","sensation","client.ini",isRFVerbose);
    iCub->opc->isVerbose = false;
    while (!iCub->connect())
    {
        cout<<"iCubClient : Some dependencies are not running..."<<endl;
        Time::delay(1.0);
    }

    homeoPort.open("/opcSensation/toHomeo:o");
    unknown_entities_port.open("/opcSensation/unknown_entities:o");
    known_entities_port.open( "/opcSensation/known_entities:o");

    yInfo() << "Configuration done.";

}

void OpcSensation::publish()
{
    Bottle res = handleEntities();

    if (!Network::isConnected("/opcSensation/toHomeo:o", "/homeostasis/fromSensations:i"))
        Network::connect("/opcSensation/toHomeo:o", "/homeostasis/fromSensations:i");
    yarp::os::Bottle &bot = homeoPort.prepare();
    bot.clear();
    yarp::os::Bottle key;
    key.clear();
    key.addString("unknown");
    key.addInt(res.get(0).asInt());
    bot.addList()=key;
    key.clear();
    key.addString("known");
    key.addInt(res.get(2).asInt());
    bot.addList()=key;
    homeoPort.write();


    yarp::os::Bottle &unkn = unknown_entities_port.prepare();
    unkn.clear();
    unkn.append(*res.get(1).asList());
    unknown_entities_port.write();

    yarp::os::Bottle &kn = known_entities_port.prepare();
    kn.clear();
    kn.append(*res.get(3).asList());
    known_entities_port.write();
}

void OpcSensation::addToEntityList(Bottle& list, string type, string name) {
    Bottle item;
    item.addString(type);
    item.addString(name);
    list.addList() = item;
}

Bottle OpcSensation::handleEntities()
{
    LockGuard lg(m_entity_bottles);
    iCub->opc->checkout();
    list<Entity*> lEntities = iCub->opc->EntitiesCache();

    k_entities.clear();
    kp_entities.clear();
    u_entities.clear();
    up_entities.clear();
    o_positions.clear();
    p_entities.clear();

    for (auto& entity : lEntities)
    {
        if(entity->entity_type() == "object") {
            Object* o = dynamic_cast<Object*>(entity);
            if(o) {
                addToEntityList(o_positions, o->objectAreaAsString(), entity->name());
            }
        }

        if (entity->name().find("unknown") == 0) {
            if (entity->entity_type() == "object")
            {
                Object* o = dynamic_cast<Object*>(entity);
                if(o && (o->m_present==1.0)) {
                    addToEntityList(up_entities, entity->entity_type(), entity->name());
                }
                addToEntityList(u_entities, entity->entity_type(), entity->name());
            }
            else if(entity->entity_type() == "bodypart") {
                addToEntityList(u_entities, entity->entity_type(), entity->name());
                addToEntityList(up_entities, entity->entity_type(), entity->name());
            }
        }
        else if (entity->entity_type() == "object")
        {
            Object* o = dynamic_cast<Object*>(entity);
            if(o && (o->m_present==1.0)) {
                addToEntityList(kp_entities, entity->entity_type(), entity->name());
            }
            addToEntityList(k_entities, entity->entity_type(), entity->name());
        }
        else if (entity->name() == "partner" && entity->entity_type() == "agent") {
            Agent* a = dynamic_cast<Agent*>(entity);
            if(a && (a->m_present==1.0)) {
                addToEntityList(up_entities, entity->entity_type(), entity->name());
            }
            addToEntityList(u_entities, entity->entity_type(), entity->name());
        }
        else {
            if (entity->entity_type() == "bodypart")
            {
                Bodypart* bp = dynamic_cast<Bodypart*>(entity);
                if(bp) {
                    if(bp->m_tactile_number == -1) {
                        addToEntityList(u_entities, entity->entity_type(), entity->name());
                        addToEntityList(up_entities, entity->entity_type(), entity->name());
                    }
                }
            }
        }
        Object * o = dynamic_cast<Object*>(entity);
        if (o && o->m_present == 1.0)
        {
            addToEntityList(p_entities, entity->entity_type(), entity->name());
        }
    }

    Bottle out;
    out.addInt(up_entities.size());
    out.addList()=up_entities;
    out.addInt(kp_entities.size());
    out.addList()=kp_entities;

    return out;
}

int OpcSensation::get_property(string name,string property)
{
    LockGuard lg(m_entity_bottles);
    Bottle b;
    bool check_position=false;

    if (property == "known")
    {
        b = k_entities;
    }
    else if (property == "unknown")
    {
        b = u_entities;
    }
    else if (property == "present")
    {
        b = p_entities;
    }
    else
    {
        b = o_positions;
        check_position=true;
    }
    if (check_position)
    {
        yDebug()<<"Checking object position"<<name<<property;
        for (unsigned int i=0;i<b.size();i++)
        {
            if (name == "any"){
                if (b.get(i).asList()->get(0).asString()==property)
                {
                    yDebug() << "check_position, name==any, return 1";
                    return 1;
                }
            }else{
                if (b.get(i).asList()->get(1).asString()==name && b.get(i).asList()->get(0).asString()==property)
                {
                    yDebug() << "check_position, name!=any, return 1 " << b.toString();
                    return 1;
                }
            }
        }
        yDebug() << "check_position, return 0";
        return 0;
    }else{
        if (name == "any"){
            if (b.size()!=0){
                yDebug() << "not check_position, name==any, return 1";
                return 1;
            }else{
                yDebug() << "not check_position, name==any, return 0";
                return 0;
            }
        }else{
            for (unsigned int i=0;i<b.size();i++)
            {
                if (b.get(i).asList()->get(1).asString()==name)
                {
                    yDebug() << "not check_position, name!=any, return 1";
                    return 1;
                }
            }
            yDebug() << "not check_position, name!=any, return 0";
            return 0;
        }
    }
}

void OpcSensation::close_ports() {
    unknown_entities_port.interrupt();
    unknown_entities_port.close();
    homeoPort.interrupt();
    homeoPort.close();
    known_entities_port.interrupt();
    known_entities_port.close();
    iCub->close();
    delete iCub;
}
