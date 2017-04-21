/* 
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Ilaria Gori and Maxime Petit
 * email:   ilaria.gori@iit.it, maxime.petit@inserm.fr
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


#include "icubclient/knowledge/action.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace icubclient;

Action::Action():Entity()
{
    m_entity_type = ICUBCLIENT_OPC_ENTITY_ACTION;
}

Action::Action(const Action &b):Entity(b)
{
    this->initialDescription = b.initialDescription;
    this->subActions = b.subActions;
    this->estimatedDriveEffects = b.estimatedDriveEffects;
}


Bottle Action::asBottle() const
{
    Bottle b = this->Entity::asBottle();
    Bottle bSub;
    bSub.addString("description");
    bSub.addList() = initialDescription.asBottle();
    b.addList() = bSub;
    bSub.clear();
    bSub.addString("subactions");
    Bottle& subs = bSub.addList();
    for(auto& subAction : subActions)
    {
        subs.addList()=subAction.asBottle();
    }
    b.addList() = bSub;
        
    bSub.clear();
    bSub.addString("estimatedDriveEffects");
    Bottle &subss = bSub.addList();
    for(auto& estimatedDriveEffect : estimatedDriveEffects)
    {
        Bottle &ss = subss.addList();
        ss.addString(estimatedDriveEffect.first.c_str());
        ss.addDouble(estimatedDriveEffect.second);
    }
    b.addList() = bSub;
    return b;
}

bool Action::fromBottle(const Bottle &b)
{
    if (!this->Entity::fromBottle(b))
        return false;

    if (!b.check("description")||!b.check("subactions"))
        return false;

    Bottle* bDesc = b.find("description").asList();
    initialDescription.fromBottle(*bDesc);

    this->subActions.clear();
    Bottle* bSub = b.find("subactions").asList();
    for(int i=0; i<bSub->size(); i++)
    {
        Action a;
        a.fromBottle(*bSub->get(i).asList());
        this->subActions.push_back(a);
    }

    this->estimatedDriveEffects.clear();
    bSub = b.find("estimatedDriveEffects").asList();
    for(int i=0; i<bSub->size(); i++)
    {
        string driveName = bSub->get(i).asList()->get(0).asString().c_str();
        double driveEffect = bSub->get(i).asList()->get(1).asDouble();
        this->estimatedDriveEffects[driveName] = driveEffect;
    }
    return true;
}

void Action::setInitialDescription(const Relation &r)
{
    this->initialDescription.fromBottle(r.asBottle());
}

Action Action::express(Relation r) const
{
    Action a;
    string initial = this->asBottle().toString().c_str();
    string final = initial.c_str();
    replace_all(final,this->initialDescription.subject().c_str(),"___subject");
    replace_all(final,this->initialDescription.object().c_str(),"___object");
    replace_all(final,this->initialDescription.complement_time().c_str(),"___compTime");
    replace_all(final,this->initialDescription.complement_place().c_str(),"___compPlace");
    replace_all(final,this->initialDescription.complement_manner().c_str(),"___compManner");
    replace_all(final,"___subject", r.subject());
    replace_all(final,"___object", r.object());
    replace_all(final,"___compTime", r.complement_time());
    replace_all(final,"___compPlace", r.complement_place());
    replace_all(final,"___compManner", r.complement_manner());
    Bottle b(final.c_str());
    a.fromBottle(b);
    return a;
}

Relation Action::description()
{
    Relation r;
    r.fromBottle(this->initialDescription.asBottle());
    return r;
}

void Action::append(Action& a)
{
    subActions.push_back(a);
}

list<Action> Action::asPlan() const
{
    list<Action> unrolled;
    if (this->subActions.size() == 0) {
        unrolled.push_back(*this);
    } else {
        for(const auto& it : subActions)
        {
            list<Action> subUnrolled = it.asPlan();
            unrolled.splice(unrolled.end(), subUnrolled);
        }
    }
    return unrolled;
}

list<Action> Action::asPlan(Relation &newDescription) const
{            
    Action expressed = this->express(newDescription);
    list<Action> unrolled = expressed.asPlan();
    return unrolled;
}

void Action::getPlanDrivesEffect(map<string,double> &driveEffects)
{
    //Added the effect of the current plan
    for(map<string, double>::iterator sIt = estimatedDriveEffects.begin() ; sIt != estimatedDriveEffects.end(); sIt++)
    {
        driveEffects[sIt->first] += sIt->second;
    }

    //Added recursively the effects of subplans
    for(list<Action>::iterator it = subActions.begin(); it != subActions.end() ; it++)
    {
        it->getPlanDrivesEffect(driveEffects);
    }
}
string Action::toString() const
{   
    return toString(this->initialDescription);
}

string Action::toString(Relation newRelation) const
{        
    std::ostringstream oss;
    oss<<"Unrolling: "<<newRelation.toString()<<endl;
    list<Action> unrolled = this->asPlan(newRelation);
    int count=0;
    for(list<Action>::iterator it = unrolled.begin() ; it!=unrolled.end(); it++,count++)
    {
        oss<<'\t'<<count<<")"<<it->description().toString()<<endl;
    }
    return oss.str();
}




