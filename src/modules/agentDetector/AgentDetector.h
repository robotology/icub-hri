/*
 * Copyright (C) 2015 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Stéphane Lallée
 * email:   stephane.lallee@gmail.com
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

#include <ctime>
#include <string>
#include <map>

#include <yarp/os/RFModule.h>
#include <yarp/math/Math.h>
#include <kinectWrapper/kinectWrapper_client.h>
#include "icubhri/clients/opcClient.h"
#include "icubhri/knowledge/agent.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::math;
using namespace kinectWrapper;
using namespace icubhri;

/**
 * @ingroup agentDetector
 */
typedef enum {
    idle,
    clicked_left,
    clicked_right } clickType;

/**
 * @ingroup agentDetector
 */
class AgentDetector: public RFModule
{
protected:
    //Kinect Related
    KinectWrapperClient     client;             //!< kinect wrapper client object
    ImageOf<PixelRgb>       rgb;
    ImageOf<PixelMono16>    depth;              //!< depth image in a Yarp format
    ImageOf<PixelFloat>     depthToDisplay;     //!< depth image to display in a Yarp format
    ImageOf<PixelBgr>       playersImage;       //!< partner image in a Yarp format
    ImageOf<PixelBgr>       skeletonImage;      //!< skeleton imange in a Yarp format
    IplImage*               depthTmp;
    IplImage*               rgbTmp;
    
    yarp::os::BufferedPort<Bottle>                                     outputSkeletonPort;  //!< Output Yarp Buffered Port of Bottle contains skeleton properties
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelFloat> > depthPort;           //!< Output Yarp Buffered Port of images contains depth information
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > imagePort;             //!< Output Yarp Buffered Port of images contains images
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelBgr> > playersPort;           //!< Output Yarp Buffered Port of partner images
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelBgr> > skeletonPort;          //!< Output Yarp Buffered Port of skeleton images
    yarp::os::BufferedPort<Bottle>                                   agentLocOutPort;       //!< Output Yarp Buffered Port of Bottle contains agent location

    double          period;
    bool            showImages;
    string          showMode;                   //!< string value of show mode of module: rgb, depth, skeleton, players
    bool            handleMultiplePlayers;      //!< boolean value for multiple players or not, if not, obtain only the closest player
    Player          joint;
    deque<Player>   joints;
    Matrix          players;
    string          partner_default_name;       //!< string value of default name of partner

    //OPC/RFH related
    OPCClient*  opc;                            //!< OPC client object
    Port        rfh;                            //!< port to referenceFrameHandler
    Port        rpc;                            //!< rpc server port to receive requests
    bool        isMounted;                      //!< boolean value to check if the IR is mounted or not
    bool        isCalibrated;                   //!< boolean value to check if the kinect is calibrated or not
    Matrix      kinect2icub;                    //!< conversion matrix from kinect frame to icub frame
    Matrix      icub2ir;
    
    int                 pointsCnt;              //!< integer of number of points
    static clickType    clicked;                //!< clicked type of object: left, right or nothing
    static float        clickX, clickY;         //!< float value of clicked coordinate in x and y axes

    //Agent Identity related
    Agent*              partner;                //!< human as an agent object
    map<int, string>    identities;
    map<string, Vector> skeletonPatterns;       //!< properties of skeleton obtained from kinect
    double              dSince;                 //!< double value of timers

    unsigned long       dTimingLastApparition;  //!< time struct of the last appartition of an agent
    double              dThresholdDisparition;  //!< timing maximal of non-reconnaissance of a agent, after thath we consider the agent as absent

    Mutex m;
    
    bool showImageParser(string &mode, string &submode);

public:

    bool configure(ResourceFinder &rf);

    bool checkCalibration();

    bool close();

    bool respond(const Bottle& cmd, Bottle& reply);

    double getPeriod();

    bool updateModule();

    /**
     * @brief setIdentity Set identity to a partner
     * @param p A kinectWrapper Player struct of partner
     * @param name string value of parner's name
     */
    void setIdentity(Player p, string name);

    /**
     * @brief getIdentity Obtain identity of partner
     * @param p A kinectWrapper Player struct of partner
     * @return string value of obtained identity
     */
    string getIdentity(Player p);

    /**
     * @brief transform2IR Transform a position to IR reference frame
     * @param v Yarp Vector of position
     * @return Yarp Vector of position in IR reference frame
     */
    Vector transform2IR(Vector v);

    /**
     * @brief getSkeletonPattern Obtain some properties of partner skeleton, such as eblow, shoulder, head position and size
     * @param p A kinectWrapper Player struct of partner
     * @return Yarp Vector contains properties
     */
    Vector getSkeletonPattern(Player p);

    static void click_callback(int event, int x, int y, int flags, void* param)
    {
        (void) flags; // to prevent unused variable warning
        (void) param; // to prevent unused variable warning

        switch (event)
        {
        case CV_EVENT_LBUTTONDOWN:
            yInfo()<<"Got a left-click.";
            AgentDetector::clickX=(float)x;
            AgentDetector::clickY=(float)y;
            AgentDetector::clicked=clicked_left;
            break;
        case CV_EVENT_RBUTTONDOWN:
            yInfo()<<"Got a right-click.";
            AgentDetector::clicked=clicked_right;
            break;
        }
    }
};

