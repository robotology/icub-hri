/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Ugo Pattacini, Stéphane Lallée
 * email:   ugo.pattacini@iit.it stephane.lallee@gmail.com
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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

#include <yarp/os/Bottle.h>
#include <string>
#include <vector>

/**
 * @defgroup icubclient_helpers Helper Routines
 *
 * @ingroup icubclient_libraries
 *
 * A container library containing defines and structures to formalize
 * knowledge within an objectPropertiesCollector module.
 *
 * @author Ugo Pattacini & Stéphane Lallée
 */

namespace icubclient{
/**
 * @ingroup icubclient_helpers
 *
 * Allow retrieving the list of unique identifiers of those items
 * verifying the set of conditions queried to the database
 * through a [ask] request.
 *
 * @param reply the bottle received from the database
 *
 * @return the list of items identifiers as a Bottle
 */
yarp::os::Bottle opcGetIdsFromAsk(const yarp::os::Bottle &reply);

/**
 * @ingroup icubclient_helpers
 *
 * Simple search and replace function for strings;
 *
 * @param in the input and output string
 * @param plain the substring to be replaced
 * @param tok the replacement
 */
void replace_all(std::string & in, const std::string & plain, const std::string & tok);

/**
 * @ingroup icubclient_helpers
 *
 * Get the context path of a .grxml grammar, and return it as a string
 *
 * @param sPath Path to the grammar file
 * @return Grammar as string
 */
std::string grammarToString(const std::string& sPath);
}

#endif
