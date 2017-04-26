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

#ifndef __ICUBCLIENT_TAGS_H__
#define __ICUBCLIENT_TAGS_H__

#include <vector>

/**
 * @defgroup icubclient_tags Global defines
 *
 * @ingroup icubclient_libraries
 *
 * Global defines which are used throughout the iCubClient library
 *
 * @author Ugo Pattacini & Stéphane Lallée & Tobias Fischer
 * @{
 */

#define ICUBCLIENT_OPC_INVALID_ID                 -1

#define ICUBCLIENT_OPC_ENTITY_TAG                 ("entity")
#define ICUBCLIENT_OPC_ENTITY_OBJECT              ("object")
#define ICUBCLIENT_OPC_ENTITY_BODYPART            ("bodypart")
#define ICUBCLIENT_OPC_ENTITY_AGENT               ("agent")
#define ICUBCLIENT_OPC_ENTITY_ACTION              ("action")
#define ICUBCLIENT_OPC_ENTITY_RELATION            ("relation")

#define ICUBCLIENT_OPC_OBJECT_NAME_TAG            ("name")
#define ICUBCLIENT_OPC_OBJECT_PRESENT_TAG         ("isPresent")
#define ICUBCLIENT_OPC_OBJECT_SALIENCY            ("saliency")
#define ICUBCLIENT_OPC_OBJECT_VALUE               ("value")
#define ICUBCLIENT_OPC_OBJECT_CURID_TAG           ("cur_id")

#define ICUBCLIENT_OPC_OBJECT_ROBOTPOS_TAG        ("position_3d")
#define ICUBCLIENT_OPC_OBJECT_ROBOTPOSX_TAG       ("robot_position_x")
#define ICUBCLIENT_OPC_OBJECT_ROBOTPOSY_TAG       ("robot_position_y")
#define ICUBCLIENT_OPC_OBJECT_ROBOTPOSZ_TAG       ("robot_position_z")
#define ICUBCLIENT_OPC_OBJECT_ROBOTORX_TAG        ("robot_orientation_x")
#define ICUBCLIENT_OPC_OBJECT_ROBOTORY_TAG        ("robot_orientation_y")
#define ICUBCLIENT_OPC_OBJECT_ROBOTORZ_TAG        ("robot_orientation_z")
#define ICUBCLIENT_OPC_OBJECT_RTDIMX_TAG          ("rt_dim_x")
#define ICUBCLIENT_OPC_OBJECT_RTDIMY_TAG          ("rt_dim_y")
#define ICUBCLIENT_OPC_OBJECT_RTDIMZ_TAG          ("rt_dim_z")


#define ICUBCLIENT_OPC_OBJECT_GUI_COLOR_R         ("color_r")
#define ICUBCLIENT_OPC_OBJECT_GUI_COLOR_G         ("color_g")
#define ICUBCLIENT_OPC_OBJECT_GUI_COLOR_B         ("color_b")
#define ICUBCLIENT_OPC_OBJECT_GUI_COLOR_ALPHA     ("color_aplha")

// OPC : Frames
#define ICUBCLIENT_OPC_FRAME_NAME                 ("frameName")
#define ICUBCLIENT_OPC_FRAME_MATRIX               ("frameMatrix")
#define ICUBCLIENT_OPC_FRAME_SCALE                ("frameScale")

// OPC : Spatial relations
#define ICUBCLIENT_OPC_OBJECT_SPATIAL_CONTAINS    ("contains")
#define ICUBCLIENT_OPC_OBJECT_SPATIAL_CONTAINED   ("isContained")
#define ICUBCLIENT_OPC_OBJECT_SPATIAL_INTERSECTS  ("intersects")

// OPC : Kinect Skeleton joints
#define ICUBCLIENT_OPC_BODY_PART_TYPE_HEAD        ("head")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_HAND_L      ("handLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_HAND_R      ("handRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_WRIST_L     ("wristLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_WRIST_R     ("wristRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_ELBOW_L     ("elbowLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_ELBOW_R     ("elbowRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_C  ("shoulderCenter")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_L  ("shoulderLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_R  ("shoulderRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_SPINE       ("spine")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_HIP_C       ("hipCenter")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_HIP_L       ("hipLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_HIP_R       ("hipRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_KNEE_L      ("kneeLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_KNEE_R      ("kneeRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_ANKLE_L     ("ankleLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_ANKLE_R     ("ankleRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_FOOT_L      ("footLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_FOOT_R      ("footRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_COLLAR_L    ("collarLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_COLLAR_R    ("collarRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_FT_L        ("fingertipLeft")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_FT_R        ("fingertipRight")
#define ICUBCLIENT_OPC_BODY_PART_TYPE_COM         ("CoM")

const std::vector<std::string> ALL_AVAILABLE_BODYPARTS = {
    ICUBCLIENT_OPC_BODY_PART_TYPE_HEAD,
    ICUBCLIENT_OPC_BODY_PART_TYPE_HAND_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_HAND_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_WRIST_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_WRIST_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_ELBOW_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_ELBOW_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_C,
    ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_SHOULDER_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_SPINE,
    ICUBCLIENT_OPC_BODY_PART_TYPE_HIP_C,
    ICUBCLIENT_OPC_BODY_PART_TYPE_HIP_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_HIP_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_KNEE_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_KNEE_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_ANKLE_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_ANKLE_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_FOOT_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_FOOT_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_COLLAR_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_COLLAR_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_FT_L,
    ICUBCLIENT_OPC_BODY_PART_TYPE_FT_R,
    ICUBCLIENT_OPC_BODY_PART_TYPE_COM
};

//Kinect tags
#define ICUBCLIENT_KINECT_ALL_INFO              ("all_info")
#define ICUBCLIENT_KINECT_DEPTH                 ("depth")
#define ICUBCLIENT_KINECT_DEPTH_PLAYERS         ("depth_players")
#define ICUBCLIENT_KINECT_DEPTH_RGB             ("depth_rgb")
#define ICUBCLIENT_KINECT_DEPTH_RGB_PLAYERS     ("depth_rgb_players")
#define ICUBCLIENT_KINECT_DEPTH_JOINTS          ("depth_joints")
#define ICUBCLIENT_KINECT_CMD_PING              ("ping")
#define ICUBCLIENT_KINECT_CMD_ACK               ("ack")
#define ICUBCLIENT_KINECT_CMD_NACK              ("nack")
#define ICUBCLIENT_KINECT_CMD_GET3DPOINT        ("get3D")
#define ICUBCLIENT_KINECT_SEATED_MODE           ("seated")
#define ICUBCLIENT_KINECT_CLOSEST_PLAYER         -1

/** @} */

#endif
