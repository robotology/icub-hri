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
 * icub-client/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#ifndef __ICUBCLIENT_AGENT_H__
#define __ICUBCLIENT_AGENT_H__

#include "object.h"
#include "relation.h"

namespace icubclient{

/**
* \ingroup icubclient_representations
*
* Represent a the body of an agent. Joints are stored as a dictionnary of string, position
*
*/
struct Body
{
    std::map<std::string, yarp::sig::Vector> m_parts;

    Body()
    {
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_HEAD].resize(3,0.0);
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_C].resize(3,0.0);
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_L].resize(3,0.0);
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_R].resize(3,0.0);
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_SPINE].resize(3,0.0);
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_HAND_R].resize(3,0.0);
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_HAND_L].resize(3,0.0);
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_ELBOW_R].resize(3,0.0);
        m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_ELBOW_L].resize(3,0.0);
        //m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_LEFT_HIP].resize(3,0.0);
        //m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_LEFT_KNEE].resize(3,0.0);
        //m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_LEFT_FOOT].resize(3,0.0);
        //m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_RIGHT_HIP].resize(3,0.0);
        //m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_RIGHT_KNEE].resize(3,0.0);
        //m_parts[ICUBCLIENT_OPC_BODY_PART_TYPE_RIGHT_FOOT].resize(3,0.0);
    }

    yarp::os::Bottle asBottle() const
    {
        yarp::os::Bottle b;
        for(auto& part : m_parts)
        {
            yarp::os::Bottle sub;
            sub.addString(part.first.c_str());
            yarp::os::Bottle position;
            position.addDouble(part.second[0]);
            position.addDouble(part.second[1]);
            position.addDouble(part.second[2]);
            sub.addList() = position;
            b.addList()= sub;
        }
        return b;
    }

    bool fromBottle(const yarp::os::Bottle &b)
    {
        for(std::map<std::string,yarp::sig::Vector>::iterator part = m_parts.begin(); part != m_parts.end(); part++)
        {
            yarp::os::Bottle* position = b.find(part->first.c_str()).asList();
            part->second[0] = position->get(0).asDouble();
            part->second[1] = position->get(1).asDouble();
            part->second[2] = position->get(2).asDouble();
        }
        return true;
    }
};

/**
* \ingroup icubclient_representations
*
* Represent an agent.
* An agent is a Object which possesses emotions and beliefs.
* Beliefs are encoded as a list of relations.
*/
class Agent : public Object
{
    friend class OPCClient;
private:
    std::list<Relation> m_belief;
public:
    Agent();
    Agent(const Agent &b);
    virtual ~Agent() {}

    std::map<std::string, double> m_emotions_intrinsic;
    Body m_body;

    virtual bool    isType(std::string _entityType) const
    {
        if (_entityType == ICUBCLIENT_OPC_ENTITY_AGENT)
            return true;
        else
            return this->Object::isType(_entityType);
    }

    virtual yarp::os::Bottle asBottle() const;
    virtual bool             fromBottle(const yarp::os::Bottle &b);
    virtual std::string      toString() const;

    /**
    * Add the belief of a relation to the agent.
    * @param r a relation that the agent will believe to be true
    */
    bool                addBelief(Relation r);

    /**
    * Remove the belief of a relation from the agent.
    * @param r a relation that the agent will believe not to be true
    */
    bool                removeBelief(Relation r);
    
    /**
    * Check if some relation is believed by an agent.
    * @param r a relation to check
    */
    bool                checkBelief(Relation r);

    /**
    * Get a read-only copy of the agent believes.
    */
    const std::list<Relation> &beliefs();

    bool                operator==(const Agent &b);
};

} //namespaces

#endif


