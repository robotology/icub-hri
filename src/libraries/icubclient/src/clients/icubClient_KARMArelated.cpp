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

#include "icubclient/clients/icubClient.h"
#include "icubclient/subsystems/subSystem_KARMA.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace icubclient;


// Left push
bool ICubClient::pushKarmaLeft(const std::string &objName, const double &targetPosYLeft,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    if (opc->isConnected())
    {
        Entity *target = opc->getEntity(objName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
        {
            yWarning() << "[iCubClient] Called pushKarmaLeft() on a unallowed entity: \"" << objName << "\"";
            return false;
        }

        Object *oTarget = dynamic_cast<Object*>(target);
        if (oTarget->m_present != 1.0)
        {
            yWarning() << "[iCubClient] Called pushKarmaLeft() on an unavailable entity: \"" << objName << "\"";
            return false;
        }

        yInfo("[icubClient pushKarmaLeft] object %s position from OPC (no calibration): %s", oTarget->name().c_str(),
            oTarget->m_ego_position.toString().c_str());
        return pushKarmaLeft(objName, oTarget->m_ego_position, targetPosYLeft, armType, options);
    }
    else
    {
        yWarning() << "[iCubClient] There is no OPC connection";
        return false;
    }
}

bool ICubClient::pushKarmaLeft(const std::string &objName, const yarp::sig::Vector &objCenter, const double &targetPosYLeft,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pushKarmaLeft() but KARMA subsystem is not available.";
        return false;
    }
    return karma->pushAside(objName,objCenter, targetPosYLeft, 0, armType, options);
}

// Right push
bool ICubClient::pushKarmaRight(const std::string &objName, const double &targetPosYRight,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    if (opc->isConnected())
    {
        Entity *target = opc->getEntity(objName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
        {
            yWarning() << "[iCubClient] Called pushKarmaLeft() on a unallowed entity: \"" << objName << "\"";
            return false;
        }

        Object *oTarget = dynamic_cast<Object*>(target);
        if (oTarget->m_present != 1.0)
        {
            yWarning() << "[iCubClient] Called pushKarmaLeft() on an unavailable entity: \"" << objName << "\"";
            return false;
        }

        yInfo("[icubClient pushKarmaRight] object %s position from OPC (no calibration): %s", oTarget->name().c_str(),
            oTarget->m_ego_position.toString().c_str());
        return pushKarmaRight(objName, oTarget->m_ego_position, targetPosYRight, armType, options);
    }
    else
    {
        yWarning() << "[iCubClient] There is no OPC connection";
        return false;
    }
}

bool ICubClient::pushKarmaRight(const std::string &objName, const yarp::sig::Vector &objCenter, const double &targetPosYRight,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pushKarmaRight() but KARMA subsystem is not available.";
        return false;
    }
    return karma->pushAside(objName, objCenter, targetPosYRight, 180, armType, options);
}

// Front push
bool ICubClient::pushKarmaFront(const std::string &objName, const double &targetPosXFront,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    if (opc->isConnected())
    {
        Entity *target = opc->getEntity(objName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
        {
            yWarning() << "[iCubClient] Called pushKarmaFront() on a unallowed entity: \"" << objName << "\"";
            return false;
        }

        Object *oTarget = dynamic_cast<Object*>(target);
        if (oTarget->m_present != 1.0)
        {
            yWarning() << "[iCubClient] Called pushKarmaFront() on an unavailable entity: \"" << objName << "\"";
            return false;
        }

        yInfo("[icubClient pushKarmaFront] object %s position from OPC (no calibration): %s", oTarget->name().c_str(),
            oTarget->m_ego_position.toString().c_str());
        return pushKarmaFront(objName, oTarget->m_ego_position, targetPosXFront, armType, options);
    }
    else
    {
        yWarning() << "[iCubClient] There is no OPC connection";
        return false;
    }
}

bool ICubClient::pushKarmaFront(const std::string& objName, const yarp::sig::Vector &objCenter, const double &targetPosXFront,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pushKarmaFront() but KARMA subsystem is not available.";
        return false;
    }
    return karma->pushFront(objName, objCenter, targetPosXFront, armType, options);
}

// Pure push in KARMA
bool ICubClient::pushKarma(const yarp::sig::Vector &targetCenter, const double &theta, const double &radius,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pushKarma() but KARMA subsystem is not available.";
        return false;
    }
    return karma->push(targetCenter, theta, radius, options);
}

// Back pull
bool ICubClient::pullKarmaBack(const std::string &objName, const double &targetPosXBack,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    if (opc->isConnected())
    {
        Entity *target = opc->getEntity(objName, true);
        if (!target->isType(ICUBCLIENT_OPC_ENTITY_OBJECT))
        {
            yWarning() << "[iCubClient] Called pushKarmaFront() on a unallowed entity: \"" << objName << "\"";
            return false;
        }

        Object *oTarget = dynamic_cast<Object*>(target);
        if (oTarget->m_present != 1.0)
        {
            yWarning() << "[iCubClient] Called pushKarmaFront() on an unavailable entity: \"" << objName << "\"";
            return false;
        }

        yInfo("[icubClient pullKarmaBack] object %s position from OPC (no calibration): %s", oTarget->name().c_str(),
            oTarget->m_ego_position.toString().c_str());
        return pullKarmaBack(objName, oTarget->m_ego_position, targetPosXBack, armType, options);
    }
    else
    {
        yWarning() << "[iCubClient] There is no OPC connection";
        return false;
    }
}

bool ICubClient::pullKarmaBack(const std::string &objName,const yarp::sig::Vector &objCenter, const double &targetPosXBack,
    const std::string &armType,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called pullKarmaBack() but KARMA subsystem is not available.";
        return false;
    }
    return karma->pullBack(objName,objCenter, targetPosXBack, armType, options);
}


// Pure pull (draw) in KARMA
bool ICubClient::drawKarma(const yarp::sig::Vector &targetCenter, const double &theta,
    const double &radius, const double &dist,
    const yarp::os::Bottle &options)
{
    SubSystem_KARMA *karma = getKARMA();
    if (karma == NULL)
    {
        yError() << "[iCubClient] Called drawKarma() but KARMA subsystem is not available.";
        return false;
    }
    return karma->draw(targetCenter, theta, radius, dist, options);
}

