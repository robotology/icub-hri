/*
* Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project ICT-612139
* Authors: Hyung Jin Chang
* email:   hj.chang@imperial.ac.uk
* website: http://wysiwyd.upf.edu/
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

#include "faceTracker.h"

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::draw;
using namespace yarp::sig::file;
using namespace yarp::dev;
using namespace std;

/************************************************************************/
bool faceTrackerModule::configure(yarp::os::ResourceFinder &rf) {

    string moduleName = rf.check("name", Value("faceTracker"), "module name (string)").asString();

    setName(moduleName.c_str());

    string xmlPath = rf.findFileByName("haarcascade_frontalface_default.xml");

    string imagePortName = rf.check("imagePortName", Value("/icub/camcalib/left/out")).asString();


    if (!handlerPort.open("/" + getName() + "/rpc")) {
        yError() << getName() << ": Unable to open port " << handlerPort.getName();
        return false;
    }

    imagePortLeft.open("/" + getName() + "/image/in");

    while(!Network::connect(imagePortName, imagePortLeft.getName()))
    {
        Time::delay(3);
        yDebug() << "try to connect left camera, please wait ...";
    }

    Property options;
    options.put("device", "remote_controlboard");
    options.put("local", "/facetracker/motor/client");
    options.put("remote", "/icub/head");

    robotHead = new PolyDriver(options);

    if(!robotHead->isValid())
    {
        yError() << "Cannot connect to the robot head";
        return false;
    }

    robotHead->view(vel);
    robotHead->view(enc);
    robotHead->view(ictrl);

    if(vel==NULL || enc==NULL || ictrl==NULL)
    {
        yError() << "Cannot get interface to robot head";
        robotHead->close();
        return false;
    }
    nr_jnts = 0;
    vel->getAxes(&nr_jnts);

    velocity_command.resize(nr_jnts);
    cur_encoders.resize(nr_jnts);
    prev_encoders.resize(nr_jnts);

    /* prepare command */
    for(int i=0;i<nr_jnts;i++)
    {
        velocity_command[i] = 0;
    }

    for(int i=0; i<nr_jnts; i++)
    {
        ictrl->setControlMode(i,VOCAB_CM_VELOCITY);
    }

    // ==================================================================
    //// create a opencv window
    cv::namedWindow("cvImage_Left",CV_WINDOW_NORMAL);

    // ==================================================================
    // face detection configuration
    face_classifier_left.load(xmlPath);

    attach(handlerPort);                  // attach to port

    // ==================================================================
    // Parameters
    counter_no_face_found = 0;
    x_buf = 0;
    y_buf = 0;

    mode = 0; // 0: going to a set position, 1: face searching, 2: face tracking, 3: face stuck,
    setpos_counter = 0;
    panning_counter = 0;
    stuck_counter = 0;

    // ==================================================================
    // random motion
    tilt_target = 0;
    pan_target = 0;

    int seed = 10000;
    srand(seed);
    pan_max = 80;
    tilt_max = 20;

    cvIplImageLeft = NULL;

    return true;
}

/************************************************************************/
bool faceTrackerModule::interruptModule() {
    yInfo() << "Interrupting module for port cleanup";
    handlerPort.interrupt();
    return true;
}

/************************************************************************/
bool faceTrackerModule::close() {
    yInfo() << "Calling close function";
    handlerPort.interrupt();
    handlerPort.close();

    cvReleaseImage(&cvIplImageLeft);
    robotHead->close();

    return true;
}

/************************************************************************/
bool faceTrackerModule::respond(const Bottle& command, Bottle& reply) {
    string helpMessage =  string(getName().c_str()) +
        " commands are: \n" +
        "help \n" +
        "quit \n" ;

    reply.clear();

    if (command.get(0).asString()=="quit") {
        reply.addString("quitting");
        return false;
    }
    else if (command.get(0).asString()=="help") {
        yInfo() << helpMessage;
        reply.addString("ok");
    }

    return true;
}

/************************************************************************/
double faceTrackerModule::getPeriod() {
    return 0.03;
}

void faceTrackerModule::moveToDefaultPosition() {
    // Going to the set position mode
    if (setpos_counter < 100)
    {
        velocity_command[0] = (0-cur_encoders[0])*0.3; // common tilt of head
        velocity_command[1] = (0-cur_encoders[1])*0.3; // common roll of head
        velocity_command[2] = (0-cur_encoders[2])*0.3; // common pan of head
        velocity_command[3] = (0-cur_encoders[3])*0.3; // common tilt of eyes
        velocity_command[4] = (0-cur_encoders[4])*0.3; // common pan of eyes

        setpos_counter++;
    }
    else
    {
        yDebug() << "Going to the set position is DONE!";

        velocity_command[0] = 0;
        velocity_command[2] = 0;
        velocity_command[3] = 0;
        velocity_command[4] = 0;

        yInfo() << "Switch to face searching mode!";
        mode = 1;
        setpos_counter = 0;
    }
    vel->velocityMove(velocity_command.data());
}

void faceTrackerModule::faceSearching(bool face_found) {
    if(face_found > 0)
    {
        yInfo() << "I found a face! Switch to face tracking mode";
        mode = 2;
        panning_counter++;
    }
    else
    {
        velocity_command[0] = (tilt_target-cur_encoders[0])*0.3;   // common tilt of head
        velocity_command[1] = (0-cur_encoders[1])*0.3; // common roll of head
        velocity_command[2] = (pan_target-cur_encoders[2])*0.3;    // common pan of head
        velocity_command[3] = (0-cur_encoders[3])*0.3; // common tilt of eyes
        velocity_command[4] = (0-cur_encoders[4])*0.3; // common pan of eyes

        if ((abs(tilt_target - cur_encoders[0]) < 1) && (abs(pan_target - cur_encoders[2]) < 1))
        {
            double pan_r = ((double)rand() / ((double)(RAND_MAX)+(double)(1)));
            double tilt_r = ((double)rand() / ((double)(RAND_MAX)+(double)(1)));

            pan_target = (int)((pan_r*pan_max)-(pan_max/2));
            tilt_target = (int)((tilt_r*tilt_max)-(tilt_max/2));
        }
    }

    vel->velocityMove(velocity_command.data());
}

void faceTrackerModule::faceTracking(const std::vector<cv::Rect> &faces_left, int biggest_face_left_idx) {
    if(faces_left.size() > 0)
    {
        double x = 320-(faces_left[biggest_face_left_idx].x + faces_left[biggest_face_left_idx].width/2);
        double y = 240-(faces_left[biggest_face_left_idx].y + faces_left[biggest_face_left_idx].height/2);

        x -= 320/2;
        y -= 240/2;

        double vx = x*0.3;  // Not to move too fast
        double vy = y*0.3;

        /* prepare command */
        for(int i=0;i<nr_jnts;i++)
        {
            velocity_command[i] = 0;
        }

        velocity_command[0] = vy;  // common tilt of head
        velocity_command[2] = vx;  // common pan of head
        velocity_command[3] = vy;  // common tilt of eyes
        velocity_command[4] = -vx; // common pan of eyes

        x_buf = x;
        y_buf = y;
        counter_no_face_found = 0;

    // stopping smoothly
    }
    else if (faces_left.size() == 0 && counter_no_face_found < 10)
    {
        double vx = x_buf*0.3;  // Not to move too fast
        double vy = y_buf*0.3;

        /* prepare command */
        for(int i=0;i<nr_jnts;i++)
        {
            velocity_command[i] = 0;
        }

        velocity_command[0] = vy;  // common tilt of head
        velocity_command[2] = vx;  // common pan of head
        velocity_command[3] = vy;  // common tilt of eyes
        velocity_command[4] = -vx; // common pan of eyes

        counter_no_face_found++;
    }
    else // faces_left.size() == 0 && counter>=10
    {
        yInfo() << "Hey! I don't see any face.";

        velocity_command[0] = 0;
        velocity_command[2] = 0;
        velocity_command[3] = 0;
        velocity_command[4] = 0;

        Time::delay(0.3);

        stuck_counter++;

        if(stuck_counter >= 10)
        {
            if(panning_counter > 5)
            {
                yDebug() << "Switch to default position mode!";
                mode = 0;
                stuck_counter = 0;
            }
            else
            {
                yDebug() << "Switch to face searching mode!";
                mode = 1;
                stuck_counter = 0;
            }
        }
        else { // for now, stay in face tracking mode
            mode = 2;
        }
    }
    vel->velocityMove(velocity_command.data());
}

int faceTrackerModule::getBiggestFaceIdx(const cv::Mat& cvMatImageLeft, const std::vector<cv::Rect> &faces_left) {
    int biggest_face_left_idx = 0;
    int biggest_face_left_size_buf = 0;

    for(unsigned int i=0; i<faces_left.size(); i++)
    {
        cv::Point flb(faces_left[i].x + faces_left[i].width,
                      faces_left[i].y + faces_left[i].height);
        cv::Point ftr(faces_left[i].x, faces_left[i].y);

        cv::rectangle(cvMatImageLeft, flb, ftr, cv::Scalar(0,255,0), 3,4,0);

        if(biggest_face_left_size_buf < faces_left[i].height)
        {
            biggest_face_left_size_buf = faces_left[i].height;
            biggest_face_left_idx = i;
        }
    }
    return biggest_face_left_idx;
}


/***************************************************************************/
bool faceTrackerModule::updateModule() {
    ImageOf<PixelRgb> *yarpImageLeft = imagePortLeft.read();

    if ( cvIplImageLeft == NULL ) {
        cvIplImageLeft = cvCreateImage(cvSize(yarpImageLeft->width(),yarpImageLeft->height()), IPL_DEPTH_8U,3);
    }

    // convert to cv::Mat
    cvCvtColor((IplImage*)yarpImageLeft->getIplImage(), cvIplImageLeft, CV_RGB2BGR);
    cv::Mat cvMatImageLeft=cv::cvarrToMat(cvIplImageLeft);

    if(yarpImageLeft!=NULL)
    {
        // resize images (downsample to 320x240)
        cv::resize(cvMatImageLeft, cvMatImageLeft, cv::Size(320, 240), 0,0,CV_INTER_NN);

        // convert captured frame to gray scale & equalize histogram
        cv::Mat cvMatGrayImageLeft;
        cv::cvtColor(cvMatImageLeft, cvMatGrayImageLeft, CV_BGR2GRAY);
        cv::equalizeHist(cvMatGrayImageLeft, cvMatGrayImageLeft);

        // face detection routine

        std::vector<cv::Rect> faces_left; // a vector array to store the face found

        face_classifier_left.detectMultiScale(cvMatGrayImageLeft, faces_left,
            1.1, // increase search scale by 10% each pass
            3,   // merge groups of three detections
            CV_HAAR_DO_CANNY_PRUNING,
            cv::Size(30,30),
            cv::Size(50,50));

        int biggest_face_left_idx = getBiggestFaceIdx(cvMatImageLeft, faces_left);

        prev_encoders = cur_encoders;
        enc->getEncoders(cur_encoders.data());

        if (mode == 0) {
            moveToDefaultPosition();
        } else if (mode == 1) {
            faceSearching(faces_left.size());
        } else if (mode == 2) {
            faceTracking(faces_left, biggest_face_left_idx);
        }
        cv::imshow("cvImage_Left", cvMatImageLeft);
    }

    cv::waitKey(1);

    return true;
}
