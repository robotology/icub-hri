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


#include <algorithm>
#include "icubclient/knowledge/agent.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace icubclient;

Agent::Agent():Object()
{    
    m_entity_type = ICUBCLIENT_OPC_ENTITY_AGENT;
    m_ego_position[0] = 0.0;
}

Agent::Agent(const Agent &b):Object(b)
{
    this->m_belief = b.m_belief;
    this->m_emotions_intrinsic = b.m_emotions_intrinsic;
    this->m_body = b.m_body;
}

Bottle Agent::asBottle() const
{
    //Get the Object bottle
    Bottle b = this->Object::asBottle();
    Bottle bSub;
    bSub.addString("belief");
    Bottle& bSubIds = bSub.addList();
    for(auto& m_belief_item : m_belief)
    {
        Bottle& bSub2 = bSubIds.addList();
        bSub2 = m_belief_item.asBottle();
    }
    b.addList() = bSub;

    Bottle bSubEmotions;
    bSubEmotions.addString("emotions");
    Bottle& bSubEmoList = bSubEmotions.addList();
    for(auto & m_emotion_intrinsic : m_emotions_intrinsic)
    {
        Bottle& bSubEmo = bSubEmoList.addList();
        bSubEmo.addString(m_emotion_intrinsic.first.c_str());
        bSubEmo.addDouble(m_emotion_intrinsic.second);
    }
    b.addList() = bSubEmotions;
              
    Bottle bSubBody;
    bSubBody.addString("body");
    bSubBody.addList() = m_body.asBottle();
    b.addList() = bSubBody;  

    return b;
}

bool Agent::fromBottle(const Bottle &b)
{
    if (!this->Object::fromBottle(b))
        return false;

    if (!b.check("belief")||!b.check("emotions"))
        return false;

    m_belief.clear();
    Bottle* beliefs = b.find("belief").asList();
    for(unsigned int i=0; i<beliefs->size() ; i++)
    {
        Bottle* bRelation = beliefs->get(i).asList();
        Relation r(*bRelation);
        m_belief.push_back(r);
    }

    m_emotions_intrinsic.clear();
    Bottle* emotions = b.find("emotions").asList();
    for(unsigned int i=0; i<emotions->size() ; i++)
    {
        Bottle* bEmo = emotions->get(i).asList();
        string emotionName = bEmo->get(0).asString().c_str();
        double emotionValue = bEmo->get(1).asDouble();
        m_emotions_intrinsic[emotionName.c_str()] = emotionValue;
    }
               
    Bottle* bodyProperty = b.find("body").asList();
    m_body.fromBottle(*bodyProperty);

    return true;
}

string Agent::toString() const
{
    std::ostringstream oss;
    oss<< this->Object::toString();

    oss<<"Believes that : \n";
    for(const auto& it : m_belief)
    {
        oss<< it.toString() <<endl;
    }

    oss<<"Emotions:"<<endl;
    for(const auto& it : m_emotions_intrinsic)
    {
        oss<< '\t'<<it.first <<" : "<<it.second<<endl;
    }

    return oss.str();
}

bool Agent::addBelief(const Relation& r)
{
    //Check if this relation is already present
    list<Relation>::const_iterator it = find(m_belief.begin(),m_belief.end(),r);
    if (it != m_belief.end())
    {
        //cout<<"Agent "+name()+" already believes that " + r.toString();
        return false;
    }

    m_belief.push_back(r);
    //cout<<"Agent "+name()+" now believes that " + r.toString();
    return true;
}

bool Agent::removeBelief(const Relation& r)
{
    //Check if this relation is already present
    list<Relation>::iterator it = find(m_belief.begin(),m_belief.end(),r);
    if (it != m_belief.end())
    {
        m_belief.erase(it);
        //cout<<"Agent "+name()+" do not believes anymore that " + r.toString();
        return true;
    }
    //cout<<"Agent "+name()+" didn not believe that " + r.toString();
    return false;
}

bool Agent::checkBelief(const Relation& r) const
{
    return (find(m_belief.begin(),m_belief.end(),r) != m_belief.end());
}

const std::list<Relation> &Agent::beliefs() const
{
    return m_belief;
}
