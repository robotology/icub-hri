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

#ifndef __ICUBHRI_OBJECT_H__
#define __ICUBHRI_OBJECT_H__

#include <yarp/sig/all.h>
#include "entity.h"
#include "icubhri/tags.h"

namespace icubhri{

enum class ObjectArea : int {HUMAN = 1, ROBOT = 2, SHARED = 3, NOTREACHABLE = 4};

/**
* \ingroup icubhri_representations
*
* Represent any physical entity (including objects and agents) that can be stored within the OPC.
*/
class Object: public Entity
{
    friend class OPCClient;

public:
    Object();
    Object(const Object &b);

    /**
    * Position of the Object, in the initial ego-centered reference frame of the agent mainting the OPC (initial root of the iCub).
    */
    yarp::sig::Vector m_ego_position;

    /**
    * Orientation of the Object, in the initial ego-centered reference frame of the agent mainting the OPC (initial root of the iCub).
    */
    yarp::sig::Vector m_ego_orientation;

    /**
    * Dimensions of the Object, in meters.
    */
    yarp::sig::Vector m_dimensions;

    /**
    * Mean color of the object (r,g,b) used mainly for debugging/displaying purposes in the iCubGUI.
    */
    yarp::sig::Vector m_color;

    /**
    * Is the object present in the scene
    * A value of 1.0 means that the object currently is in the scene
    * A value of 0.5 means that the object is likely to be in the scene, but not currently visible by the iCub
    * A value of 0.0 means that the object is currently not in the scene
    */
    double m_present;

    /**
    * A measurement of the object saliency [0,1]
    */
    double m_saliency;

    /**
    * Whether the object is accessible by only the robot, only the human, both or neither agent
    */
    ObjectArea m_objectarea;

    /**
    * A measurement of the object subjective value [0,1]
    */  
    double m_value;

    virtual bool    isType(std::string _entityType) const
    {
        if (_entityType == ICUBHRI_OPC_ENTITY_OBJECT) {
            return true;
        } else {
            return this->Entity::isType(_entityType);
        }
    }

    virtual yarp::os::Bottle asBottle() const;
    virtual bool             fromBottle(const yarp::os::Bottle &b);
    virtual std::string      toString() const;

    /**
    * Express a point given in the initial ego-centered reference frame in respect to the object reference frame.
    *@param vInitialRoot the point to transform
    *@return The transformed vector (x y z)
    */
    yarp::sig::Vector getSelfRelativePosition(const yarp::sig::Vector &vInitialRoot) const;

    std::string objectAreaAsString() const {
        return objectAreaAsString(m_objectarea);
    }

    static std::string objectAreaAsString(const icubhri::ObjectArea &o);
    static icubhri::ObjectArea stringToObjectArea(const std::string &o);

    /**
     * @brief Get saliency of object
     * @return saliency of object
     */
    double saliency() const;

    /**
     * @brief Set saliency of object
     * @param saliency - New saliency value
     */
    void setSaliency(double saliency);

    /**
     * @brief Get value of object
     * @return value of object
     */
    double value() const;

    /**
     * @brief Set value of object
     * @param value - New value
     */
    void setValue(double value);
};

} //namespace

#endif

