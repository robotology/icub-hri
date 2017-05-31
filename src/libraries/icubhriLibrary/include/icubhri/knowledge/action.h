/* 
* Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
* Authors: Gori Ilaria and Petit Maxime
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

#ifndef __ICUBHRI_ACTION_H__
#define __ICUBHRI_ACTION_H__

#include "entity.h"
#include "relation.h"
#include "icubhri/tags.h"

namespace icubhri{

    /**
    * \ingroup icubhri_representations
    *
    * Represents an action, whether composite or not. Stores as well the expected impact on the drives.
    */
    class Action:public Entity
    {
        friend class OPCClient;

        Relation initialDescription; /**< The description used when the action was teached */
        std::list<Action> subActions; /**< Subactions that may compose a composite one */

    public:
        /**
        * Estimated effects on the drive
        */
        std::map<std::string, double> estimatedDriveEffects;

        Action();
        Action(const Action &b);

        virtual bool isType(std::string _entityType) const
        {
            if (_entityType == ICUBHRI_OPC_ENTITY_ACTION)
                return true;
            else
                return this->Entity::isType(_entityType);
        }

        void setInitialDescription(const Relation &r);
        Relation    description() const; //!< Return initialDescription as Relation
        Action      express(const Relation &r) const;

        /**
        * Append a subaction to create a composite one
        * @param a The subaction
        */ 
        void    append(Action &a);

        /**
        * Get an unrolled plan description
        */ 
        virtual std::list<Action>    asPlan() const;

        /**
        * Get an unrolled plan description based on new arguments
        * @param newDescription The new arguments
        */ 
        virtual std::list<Action>    asPlan(const Relation& newDescription) const;

        /**
        * Is an action composite or not?
        */ 
        virtual bool isComposite(){ return subActions.size(); }

        /**
        * Number of subactions composing this one
        */ 
        int size(){ return subActions.size(); }

        /**
        * Number of subactions composing this one
        */
        virtual yarp::os::Bottle asBottle() const;
        virtual bool             fromBottle(const yarp::os::Bottle &b);
        virtual std::string      toString() const;
                
        /**
        * Estimate the effect of this plan on a specific drive, by summing all the effects of
        * subplans to this specific plan effect.
        */ 
        virtual void getPlanDrivesEffect(std::map<std::string,double> &driveEffects);

        /**
        * Gives the string according to a current situation
        * @param newRelation The new arguments
        */ 
        virtual std::string toString(const Relation &newRelation) const;
    };

}//Namespace

#endif

