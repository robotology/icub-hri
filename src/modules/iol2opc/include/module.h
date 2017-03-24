/*
 * Copyright (C) 2015 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
 * Authors: Ugo Pattacini, Tobias Fischer
 * email:   ugo.pattacini@iit.it, t.fischer@imperial.ac.uk
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * wysiwyd/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#ifndef __MODULE_H__
#define __MODULE_H__

#include <string>
#include <deque>
#include <map>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>
#include <yarp/math/Rand.h>
#include <iCub/ctrl/filters.h>
#include <wrdac/clients/opcClient.h>

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

#include "utils.h"
#include "iol2opc_IDL.h"

#define RET_INVALID     -1
#define OBJECT_UNKNOWN  "?"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::math;
using namespace iCub::ctrl;
using namespace wysiwyd::wrdac;


/**********************************************************/
namespace Bridge {
    typedef enum { idle, load_database, localization } State;
}


/**********************************************************/
class IOLObject
{
protected:
    MedianFilter filterPos;
    MedianFilter filterDim;
    bool init_filters;
    double presenceTmo;
    double presenceTimer;

    enum { idle, init, no_need, tracking };

    string trackerType;
    int trackerState;
    double trackerTmo;
    double trackerTimer;    

    cv::Rect2d trackerResult;
    cv::Ptr<cv::Tracker> tracker;

public:
    /**********************************************************/
    IOLObject(const int filter_order=1, const double presenceTmo_=0.0,
              const string &trackerType_="BOOSTING", const double trackerTmo_=0.0) :
              filterPos(filter_order), filterDim(10*filter_order),
              init_filters(true), presenceTmo(presenceTmo_),
              trackerType(trackerType_), trackerState(idle),
              trackerTmo(trackerTmo_), trackerTimer(0.0)              
    {
        trackerResult.x=trackerResult.y=0;
        trackerResult.width=trackerResult.height=0;
        heartBeat();
    }

    /**********************************************************/
    void heartBeat()
    {
        presenceTimer=Time::now();
    }

    /**********************************************************/
    bool isDead()
    {
        bool dead=(Time::now()-presenceTimer>=presenceTmo);
        if (dead)
            trackerState=idle;
        return dead;
    }

    /**********************************************************/
    void filt(const Vector &x, Vector &xFilt,
              const Vector &d, Vector &dFilt)
    {
        if (init_filters)
        {
            filterPos.init(x); xFilt=x;
            filterDim.init(d); dFilt=d;
            init_filters=false;
        }
        else
        {
            xFilt=filterPos.filt(x);
            dFilt=filterDim.filt(d);
        }
    }

    /**********************************************************/
    void prepare()
    {
        if (trackerState==no_need)
            trackerState=init;
    }

    /**********************************************************/
    void latchBBox(const CvRect& bbox)
    {
        trackerResult.x=bbox.x;
        trackerResult.y=bbox.y;
        trackerResult.width=bbox.width;
        trackerResult.height=bbox.height;
        trackerState=no_need;
    }

    /**********************************************************/
    void track(const Image& img)
    {
        cv::Mat frame=cv::cvarrToMat((IplImage*)img.getIplImage());
        if (trackerState==init)
        {
            tracker=cv::Tracker::create(trackerType);
            tracker->init(frame,trackerResult);
            trackerTimer=Time::now();
            trackerState=tracking;
        }
        else if (trackerState==tracking)
        {
            if (Time::now()-trackerTimer<trackerTmo)                
            {
                tracker->update(frame,trackerResult);
                CvPoint tl=cvPoint((int)trackerResult.x,(int)trackerResult.y);
                CvPoint br=cvPoint(tl.x+(int)trackerResult.width,
                                   tl.y+(int)trackerResult.height);

                if ((tl.x<5) || (br.x>frame.cols-5) ||
                    (tl.y<5) || (br.y>frame.rows-5))
                    trackerState=idle;                    
                else
                    heartBeat();                    
            }
            else
                trackerState=idle;
        }
    }

    /**********************************************************/
    bool is_tracking(CvRect& bbox) const
    {
        bbox=cvRect((int)trackerResult.x,(int)trackerResult.y,
                    (int)trackerResult.width,(int)trackerResult.height);
        return (trackerState!=idle);
    }
};


/**********************************************************/
class IOL2OPCBridge : public RFModule, public iol2opc_IDL
{
protected:
    RpcServer  rpcPort;
    RpcClient  rpcClassifier;
    RpcClient  rpcGet3D;
    OPCClient *opc;

    BufferedPort<Bottle>             blobExtractor;
    BufferedPort<Bottle>             histObjLocPort;
    BufferedPort<Bottle>             getClickPort;
    BufferedPort<Bottle>             objLocOut;
    BufferedPort<ImageOf<PixelBgr> > imgIn;
    BufferedPort<ImageOf<PixelBgr> > imgRtLocOut;
    BufferedPort<ImageOf<PixelBgr> > imgTrackOut;
    BufferedPort<ImageOf<PixelBgr> > imgSelBlobOut;
    BufferedPort<ImageOf<PixelBgr> > imgHistogram;
    Port imgClassifier;

    RtLocalization rtLocalization;
    OpcUpdater opcUpdater;
    int opcMedianFilterOrder;
    ClassifierReporter classifierReporter;

    ImageOf<PixelBgr> imgRtLoc;
    Mutex mutexResources;
    Mutex mutexResourcesOpc;
    Mutex mutexResourcesSFM;

    double period;
    bool verbose;
    bool empty;
    bool object_persistence;

    double presence_timeout;
    string tracker_type;
    double tracker_timeout;
    VectorOf<int> tracker_min_blob_size;
    map<string,IOLObject> db;
    Bridge::State state;
    IOLObject onlyKnownObjects;

    map<string,Filter*> histFiltersPool;
    int histFilterLength;
    deque<CvScalar> histColorsCode;

    double blobs_detection_timeout;
    double lastBlobsArrivalTime;
    Bottle lastBlobs;
    Bottle opcBlobs;
    Bottle opcScores;

    Vector skim_blobs_x_bounds;
    Vector skim_blobs_y_bounds;
    Vector histObjLocation;

    Vector human_area_x_bounds;
    Vector human_area_y_bounds;
    Vector robot_area_x_bounds;
    Vector robot_area_y_bounds;
    Vector shared_area_x_bounds;
    Vector shared_area_y_bounds;

    CvPoint clickLocation;

    friend class RtLocalization;
    friend class OpcUpdater;
    friend class ClassifierReporter;

    void    yInfoGated(const char *msg, ...) const;

    /**
     * @brief findName Find name of a blob or set it OBJECT_UNKNOWN
     * @param scores Bottle contains classified results from himrepClassifier, @see classify()
     * @param tag input string contains blob name, i.e. "blob_<i>" with i as index of a blob in set of results from getBlobs()
     * @return string value for name of the blob: OBJECT_UNKNOWN or class name from classifier
     */
    string  findName(const Bottle &scores, const string &tag);

    /**
     * @brief skimBlobs Remove blobs which are too far away in the Cartesian space wrt Root frame
     * @param blobs Original Yarp Bottle contains all blobs
     * @return Result Bottle contains valid blobs
     */
    Bottle  skimBlobs(const Bottle &blobs);

    /**
     * @brief thresBBox Constrain a bounding box with respect to an image size
     * @param bbox CvRect define the bounding box
     * @param img Image with information used to constrain the bounding box
     * @return True if the size of the constrained bounding box is large enough (bigger than tracker_min_blob_size)
     */
    bool    thresBBox(CvRect &bbox, const Image &img);

    /**
     * @brief getBlobs Wrapper to get blobs information from the blobExtractor port
     * @return Yarp Bottle contains all valid segmented object blobs from lbpExtract module
     */
    Bottle  getBlobs();

    /**
     * @brief getBlobCOG Get the blob center of the object defined by i
     * @param blobs Yarp Bottle contains information of the estimated object bounding boxes: top left x, y, bottom right x, y
     * @param i integer value to define object
     * @return A CvPoint containing x, y coordinate of the blob center
     */
    CvPoint getBlobCOG(const Bottle &blobs, const int i);

    /**
     * @brief get3DPosition Get the 3D point coordinate in Root frame through SFM
     * @param point A CvPoint for the estimated coordinate of an image pixel.
     * @param x Vector for 3D point coordinate. If SFM return more than one 3D point as result, the coordinate is average of all result
     * @return True if can get 3D point from SFM, False otherwise
     */
    bool    get3DPosition(const CvPoint &point, Vector &x);

    /**
     * @brief get3DPositionAndDimensions Get the 3D point coordinate and dimension in Root frame through SFM
     * @param bbox A CvRect for bounding box of object
     * @param x Vector for 3D point coordinate. If SFM return more than one 3D point as result, the coordinate is average of all result
     * @param dim Vector for 3D dimensions of object, calculated using coordinates of points from SFM
     * @return True if get response from SFM, False otherwise
     */
    bool    get3DPositionAndDimensions(const CvRect &bbox, Vector &x, Vector &dim);
    Vector  calibPosition(const Vector &x);

    /**
     * @brief getClickPosition Get the position on image that user clicks on
     * @param pos A CvPoint contains 2D coordinate of the estimated pixel that user clicks on
     * @return True if the result pixel is valid, False otherwise
     */
    bool    getClickPosition(CvPoint &pos);

    /**
     * @brief acquireImage Get calibrated image from the left camera of iCub
     */
    void    acquireImage();

    /**
     * @brief drawBlobs Add bounding box and name of object in output image shown through port imgRtLocOut
     * @param blobs Yarp Bottle contains information of the estimated object bounding boxes: top left x, y, bottom right x, y
     * @param i integer value as index of the blob to draw the bounding box
     * @param scores Bottle contains classified results from himrepClassifier, @see classify()
     */
    void    drawBlobs(const Bottle &blobs, const int i, const Bottle &scores);

    /**
     * @brief rotate Create new OpenCV matrix by rotating an original one an angle in 2D, a wrapper of cv::warpAffine
     * @param src Original OpenCV matrix
     * @param angle Double value for rotated angle
     * @param dst New rotated OpenCV matrix
     */
    void    rotate(cv::Mat &src, const double angle, cv::Mat &dst);

    /**
     * @brief drawScoresHistogram Draw histogram of objects' scores
     * @param blobs Yarp Bottle contains information of the estimated object bounding boxes: top left x, y, bottom right x, y
     * @param i integer value as index of the blob to draw the bounding box
     * @param scores Bottle contains classified results from himrepClassifier, @see classify()
     */
    void    drawScoresHistogram(const Bottle &blobs, const Bottle &scores, const int i);

    /**
     * @brief findClosestBlob Find the closest blob to location
     * @param blobs Yarp Bottle contains information of the estimated object bounding box: top left x, y, bottom right x, y
     * @param loc A CvPoint contains 2D coordinate of pixel expressed the location
     * @return integer value of the corresponding blob to the location
     */
    int     findClosestBlob(const Bottle &blobs, const CvPoint &loc);

    /**
     * @brief findClosestBlob Find the closest blob to location
     * @param blobs Yarp Bottle contains information of the estimated object bounding boxes: top left x, y, bottom right x, y
     * @param loc A Vector contains 3D coordinate of the location in the Root frame
     * @return integer value of the corresponding blob to the location
     */
    int     findClosestBlob(const Bottle &blobs, const Vector &loc);

    /**
     * @brief classify Classify an object using himrepClassifier
     * @param blobs Yarp Bottle contains information of the estimated object bounding boxes: top left x, y, bottom right x, y. This will be sent to himrepClassifier for object detection
     * @return Yarp Bottle contains class of object and confidence score
     */
    Bottle  classify(const Bottle &blobs);

    /**
     * @brief train Training himrepClassifier by sending an object by name and its bounding box
     * @param object string value contains name of object to train
     * @param blobs Yarp Bottle contains information of the estimated object bounding boxes: top left x, y, bottom right x, y
     * @param i integer value for the corresponding blob of the object needed to train
     */
    void    train(const string &object, const Bottle &blobs, const int i);

    /**
     * @brief doLocalization The whole procedure to classify objects, from getting image to draw bounding boxes arround objects
     */
    void    doLocalization();

    /**
     * @brief updateOPC The whole procedure to update OPC, including naming known and unknown objects, drawing bounding boxes around objects
     */
    void    updateOPC();

    /**
     * @brief getReachableArea Analyse if an object is inside which region: Human, Shared, Robot or Not-Reachable
     * @param objpos Vector of object position
     * @return ObjectArea that the object belongs to
     */
    ObjectArea getReachableArea(const yarp::sig::Vector &objpos);

    bool    configure(ResourceFinder &rf);

    /**
     * @brief setBounds Set min & max for a vector
     * @param rf ResourceFinder object
     * @param bounds Vector to set bounds to
     * @param configName string value for name of the parameter in config file
     * @param std_lower double value for min bound
     * @param std_upper double value for max bound
     */
    void    setBounds(ResourceFinder &rf, Vector &bounds, string configName, double std_lower, double std_upper);
    bool    interruptModule();
    bool    close();
    bool    attach(RpcServer &source);
    double  getPeriod();
    bool    updateModule();

public:
    bool    train_object(const string &name);
    bool    remove_object(const string &name);
    bool    remove_all();
    bool    change_name(const string &old_name, const string &new_name);
    bool    set_object_persistence(const string &sw);
    string  get_object_persistence();
    void    pause();
    void    resume();
};

#endif

