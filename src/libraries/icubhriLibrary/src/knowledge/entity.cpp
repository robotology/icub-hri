/* 
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Stéphane Lallée
 * email:   stephane.lallee@gmail.com
 * website: https://github.com/robotology/icub-client/ 
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * icub-hri/LICENSE
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include "icubhri/knowledge/entity.h"

using namespace std;
using namespace yarp::os;
using namespace icubhri;

Entity::Entity(yarp::os::Bottle &b)
{
    this->m_opc_id = -1;
    this->fromBottle(b);
}

Entity::Entity()
{
    this->m_opc_id = -1;
    this->m_entity_type = "unknown";
    this->m_name = "";
}

Entity::Entity(const Entity &b)
{
    this->m_name = b.m_name;
    this->m_entity_type = b.m_entity_type;
    this->m_opc_id = b.m_opc_id;
}

Bottle Entity::asBottle() const
{
    Bottle b;

    Bottle bName;
    bName.addString("name");
    bName.addString(m_name.c_str());
    b.addList() = bName;

    Bottle bEntity;
    bEntity.addString("entity");
    bEntity.addString(m_entity_type.c_str());
    b.addList() = bEntity;
        
    Bottle bProperties;
    bProperties.addString("intrinsic_properties");
    Bottle& bPropertiesGlob = bProperties.addList();
    for(auto& m_property : m_properties)
    {
        Bottle bPropertiesValues;
        bPropertiesValues.addString(m_property.first.c_str());
        bPropertiesValues.addString(m_property.second.c_str());
        bPropertiesGlob.addList() = bPropertiesValues;
    }
    b.addList() = bProperties;
    return b;
}

Bottle Entity::asBottleOnlyModifiedProperties() const
{
    Bottle current_entity = this->asBottle();

    Bottle new_entity;
    for(int p=0; p<current_entity.size(); p++)
    {
        string currentTag = current_entity.get(p).asList()->get(0).asString();
        Value &currentValue = current_entity.find(currentTag.c_str());
        Value &originalValue = m_original_entity.find(currentTag.c_str());

        if (currentValue.toString() != originalValue.toString())
        {
            Bottle &bSub=new_entity.addList();
            bSub.addString(currentTag);
            bSub.add(currentValue);
        }
    }

    return new_entity;
}

bool Entity::fromBottle(const Bottle &b)
{
    if (!b.check("name") || !b.check("entity"))
        return false;

//  m_opc_id = b->find("id").asInt();
    m_name = b.find("name").asString().c_str();
    m_entity_type = b.find("entity").asString().c_str();
        
    Bottle* bProperties = b.find("intrinsic_properties").asList();
    
    for(int i=0; i< bProperties->size() ; i++)
    {
        std::string pTag = bProperties->get(i).asList()->get(0).asString().c_str();
        std::string pVal = bProperties->get(i).asList()->get(1).asString().c_str();
        m_properties[pTag] = pVal;
    }

    return true;
}

string Entity::toString() const
{
    std::ostringstream oss;
    oss << "**************"<<endl;
    oss <<"Name : " <<    m_name<<endl;
    oss <<"ID : "    <<    m_opc_id<<endl;
    oss <<"Type : "    <<    m_entity_type<<endl;
    oss <<"Intrinsic properties: "<<endl;
    for(const auto& prop : m_properties) {
        oss<<'\t'<<prop.first<<" : "<<prop.second<<endl;
    }
    return oss.str();
}


bool Entity::operator==(const Entity &b)
{
    return (this->m_opc_id == b.m_opc_id);
}


bool Entity::operator<(const Entity &b)
{
    return (this->m_opc_id < b.m_opc_id );
}


bool Entity::operator>(const Entity &b)
{
    return (this->m_opc_id > b.m_opc_id );
}


void Entity::changeName(std::string sName)
{
    m_name = sName;
}
