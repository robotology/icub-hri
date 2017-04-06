#include <algorithm>    // std::random_shuffle
#include "opcSensation.h"

void OpcSensation::configure(yarp::os::ResourceFinder &rf)
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
    opc_has_agent_port.open("/opcSensation/hasAgent:o");
    known_entities_port.open( "/opcSensation/known_entities:o");
    is_touched_port.open("/opcSensation/is_touched:o"); //needed?

    u_entities.clear();
    k_entities.clear();

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

    yarp::os::Bottle &has_ag = opc_has_agent_port.prepare();
    has_ag.clear();
    has_ag.addInt(int(res.get(4).asInt()));
    opc_has_agent_port.write();
    
    handleTouch();       
 
}

void OpcSensation::handleTouch()
{
    list<Relation> tactileRelations = iCub->opc->getRelationsMatching("icub","is","any","touchLocation");
    Bottle& out = is_touched_port.prepare();
    out.clear();
    out.addInt(int(tactileRelations.size() > 0));  // is touched
    is_touched_port.write();
}

void OpcSensation::addToEntityList(Bottle& list, string type, string name) {
    Bottle item;
    item.addString(type);
    item.addString(name);
    list.addList() = item;
}

Bottle OpcSensation::handleEntities()
{
    iCub->opc->checkout();
    list<Entity*> lEntities = iCub->opc->EntitiesCache();

    bool agentPresent = false;
    Bottle temp_u_entities, temp_k_entities, temp_up_entities, temp_kp_entities, temp_p_entities, temp_o_positions;
    Bottle objects;

    for (auto& entity : lEntities)
    {
        if(entity->entity_type() == "object") {
            Object* o = dynamic_cast<Object*>(entity);
                if(o) {
                    addToEntityList(temp_o_positions, o->objectAreaAsString(), entity->name());
                }
        

        }

        if (entity->name().find("unknown") == 0) {
            if (entity->entity_type() == "object")
            {
                Object* o = dynamic_cast<Object*>(entity);
                
                if(o && (o->m_present==1.0)) {
                    addToEntityList(temp_up_entities, entity->entity_type(), entity->name());
                }
                addToEntityList(temp_u_entities, entity->entity_type(), entity->name());
            }
            else if(entity->entity_type() == "bodypart") {
                addToEntityList(temp_u_entities, entity->entity_type(), entity->name());
                addToEntityList(temp_up_entities, entity->entity_type(), entity->name());
            }
        }
        else if (entity->name() == "partner" && entity->entity_type() == "agent") {
            Agent* a = dynamic_cast<Agent*>(entity);
            if(a && (a->m_present==1.0)) {
                addToEntityList(temp_up_entities, entity->entity_type(), entity->name());
            }
            addToEntityList(temp_u_entities, entity->entity_type(), entity->name());
        }
        else {
            if (entity->entity_type() == "bodypart" && (dynamic_cast<Bodypart*>(entity)->m_tactile_number == -1))
            {
                addToEntityList(temp_u_entities, entity->entity_type(), entity->name());
                addToEntityList(temp_up_entities, entity->entity_type(), entity->name());
            }
        }
        if (dynamic_cast<Object*>(entity) && dynamic_cast<Object*>(entity)->m_present == 1.0)
        {
            addToEntityList(temp_p_entities, entity->entity_type(), entity->name());
        }
    }

    u_entities.copy( temp_u_entities);
    k_entities.copy( temp_k_entities);
    p_entities.copy( temp_p_entities);
    up_entities.copy( temp_up_entities);
    kp_entities.copy( temp_kp_entities);
    o_positions.copy( temp_o_positions);

    Bottle out;
    out.addInt(up_entities.size());
    out.addList()=up_entities;
    out.addInt(kp_entities.size());
    out.addList()=kp_entities;
    out.addInt(int(agentPresent));

    return out;
}

int OpcSensation::get_property(string name,string property)
{
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
        yDebug()<<"b:"<<b.toString();
        for (int i=0;i<b.size();i++)
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
            for (int i=0;i<b.size();i++)
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

