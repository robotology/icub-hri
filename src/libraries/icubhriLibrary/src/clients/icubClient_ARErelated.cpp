/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Stéphane Lallée, Tobias Fischer
 * email:   stephane.lallee@gmail.com, t.fischer@imperial.ac.uk
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

#include "icubhri/clients/icubClient.h"
#include "icubhri/subsystems/subSystem_ARE.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace icubhri;

bool ICubClient::home(const string &part)
{
    SubSystem_ARE *are = getARE();
    if (are == nullptr)
    {
        yError() << "[iCubClient] Called home() but ARE subsystem is not available.";
        return false;
    }

    return are->home(part);
}

bool ICubClient::release(const std::string &oLocation, const yarp::os::Bottle &options)
{
    Entity *target = opc->getEntity(oLocation, true);
    if (!target || !target->isType(ICUBHRI_OPC_ENTITY_OBJECT))
    {
        yError() << "[iCubClient] Called release() on a unallowed location: \"" << oLocation << "\"";
        return false;
    }

    Object *oTarget = dynamic_cast<Object*>(target);
    if (!oTarget || oTarget->m_present != 1.0)
    {
        yWarning() << "[iCubClient] Called release() on an unavailable entity: \"" << oLocation << "\"";
        return false;
    }

    return release(oTarget->m_ego_position, options);
}


bool ICubClient::release(const yarp::sig::Vector &target, const yarp::os::Bottle &options)
{
    SubSystem_ARE *are = getARE();
    if (are == nullptr)
    {
        yError() << "[iCubClient] Called release() but ARE subsystem is not available.";
        return false;
    }

    if (isTargetInRange(target))
        return are->dropOn(target, options);
    else
    {
        yWarning() << "[iCubClient] Called release() on a unreachable location: (" << target.toString(3, 3).c_str() << ")";
        return false;
    }
}

bool ICubClient::waving(const bool sw) {
    SubSystem_ARE *are = getARE();
    if (are == nullptr)
    {
        yError() << "[iCubClient] Called waving() but ARE subsystem is not available.";
        return false;
    }

    return are->waving(sw);
}


bool icubhri::ICubClient::point(const yarp::sig::Vector &target, const yarp::os::Bottle &options)
{
    SubSystem_ARE *are = getARE();
    if (are == nullptr)
    {
        yError() << "[iCubClient] Called pointfar() but ARE subsystem is not available.";
        return false;
    }

    Bottle opt(options);
    opt.addString("still"); // always avoid automatic homing after point
    return are->point(target, opt);
}


bool icubhri::ICubClient::point(const std::string &sName, const yarp::os::Bottle &options)
{
    Entity *target = opc->getEntity(sName, true);
    if (!target || (!target->isType(ICUBHRI_OPC_ENTITY_OBJECT) && !target->isType(ICUBHRI_OPC_ENTITY_BODYPART)))
    {
        yWarning() << "[iCubClient] Called point() on a unallowed location: \"" << sName << "\"";
        return false;
    }

    Object *oTarget = dynamic_cast<Object*>(target);
    if(oTarget!=nullptr) {
        SubSystem_ARE *are = getARE();
        if(are) {
            Vector target = oTarget->m_ego_position;
            are->selectHandCorrectTarget(const_cast<Bottle&>(options), target, sName);
            return point(target, options);
        } else {
            yError() << "[iCubClient] Called point() but ARE subsystem is not available.";
            return false;
        }
    } else {
        yError() << "[iCubClient] pointfar: Could not cast Entity to Object";
        return false;
    }
}


bool ICubClient::push(const string &oLocation, const Bottle &options)
{
    SubSystem_ARE *are = getARE();
    if (are == nullptr)
    {
        yError() << "[iCubClient] Called push() but ARE subsystem is not available.";
        return false;
    }

    Bottle opt(options);
    opt.addString("still"); // always avoid automatic homing after point

    return are->push(oLocation, opt);
}


bool ICubClient::take(const std::string& oName, const Bottle &options)
{
    SubSystem_ARE *are = getARE();
    if (are == nullptr)
    {
        yError() << "[iCubClient] Called take() but ARE subsystem is not available.";
        return false;
    }

    Bottle opt(options);
    opt.addString("still"); // always avoid automatic homing after point
    return are->take(oName, opt);
}


bool ICubClient::look(const std::string &target, const yarp::os::Bottle &options)
{
    if (SubSystem_ARE *are = getARE())
    {
        if (Object *oTarget = dynamic_cast<Object*>(opc->getEntity(target, true)))
            if (oTarget->m_present == 1.0)
                return are->look(oTarget->m_ego_position, options);

        yWarning() << "[iCubClient] Called look() on an unavailable target: \"" << target << "\"";
        return false;
    }

    yError() << "Error, neither Attention nor ARE are running...";
    return false;
}

bool ICubClient::look(const yarp::sig::Vector &target, const yarp::os::Bottle &options)
{
    if (SubSystem_ARE *are = getARE())
    {
        return are->look(target, options);
    }

    yError() << "Error, ARE not running...";
    return false;
}

bool ICubClient::lookAtBodypart(const std::string &sBodypartName)
{
    if (SubSystem_ARE *are = getARE())
    {
        Vector vLoc;
        vLoc = getPartnerBodypartLoc(sBodypartName);
        if (vLoc.size() == 3){
            return are->look(vLoc, yarp::os::Bottle());
        }

        yWarning() << "[iCubClient] Called lookAtBodypart() on an unvalid/unpresent agent or bodypart (" << sBodypartName << ")";
        return false;
    }
    return false;
}
