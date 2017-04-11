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

#include <fstream>
#include <yarp/os/Value.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Vocab.h>
#include "icubclient/functions.h"

using namespace yarp::os;
using namespace icubclient;

/************************************************************************/
yarp::os::Bottle icubclient::opcGetIdsFromAsk(const yarp::os::Bottle &reply)
{
    Bottle ids;
    if (reply.size()>0)
        if (reply.get(0).asVocab()==Vocab::encode("ack"))
            if (Bottle *idProp=reply.get(1).asList())
                if (Bottle *idList=idProp->get(1).asList())
                    ids=*idList;

    return ids;
}

/************************************************************************/
void icubclient::replace_all(std::string & in, const std::string & plain, const std::string & tok)
{
    std::string::size_type n = 0;
    const std::string::size_type l = plain.length();
    while(1){
        n = in.find(plain, n);
        if(n != std::string::npos){
            in.replace(n, l, tok);
        }
        else{
            break;
        }
    }
}

/************************************************************************/
std::string icubclient::grammarToString(const std::string &sPath)
{
    std::string sOutput = "";
    std::ifstream isGrammar(sPath.c_str());

    yDebug() << "Path is:" << sPath;

    if (!isGrammar)
    {
        yError() << "grammarToString. Couldn't open file :" << sPath;
        return "Error in grammarToString. Couldn't open file";
    }

    std::string sLine;
    while (getline(isGrammar, sLine))
    {
        sOutput += sLine;
        sOutput += "\n";
    }

    return sOutput;
}
