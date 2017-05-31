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

#ifndef __ICUBHRI_TAGS_H__
#define __ICUBHRI_TAGS_H__

#include <vector>

/**
 * @defgroup icubhri_tags Global defines
 *
 * @ingroup icubhri_libraries
 *
 * Global defines which are used throughout the iCubClient library
 *
 * @author Ugo Pattacini & Stéphane Lallée & Tobias Fischer
 * @{
 */

#define ICUBHRI_OPC_INVALID_ID                 -1

#define ICUBHRI_OPC_ENTITY_TAG                 ("entity")
#define ICUBHRI_OPC_ENTITY_OBJECT              ("object")
#define ICUBHRI_OPC_ENTITY_BODYPART            ("bodypart")
#define ICUBHRI_OPC_ENTITY_AGENT               ("agent")
#define ICUBHRI_OPC_ENTITY_ACTION              ("action")
#define ICUBHRI_OPC_ENTITY_RELATION            ("relation")

#define ICUBHRI_OPC_OBJECT_NAME_TAG            ("name")
#define ICUBHRI_OPC_OBJECT_PRESENT_TAG         ("isPresent")
#define ICUBHRI_OPC_OBJECT_SALIENCY            ("saliency")
#define ICUBHRI_OPC_OBJECT_VALUE               ("value")
#define ICUBHRI_OPC_OBJECT_CURID_TAG           ("cur_id")

#define ICUBHRI_OPC_OBJECT_ROBOTPOS_TAG        ("position_3d")
#define ICUBHRI_OPC_OBJECT_ROBOTPOSX_TAG       ("robot_position_x")
#define ICUBHRI_OPC_OBJECT_ROBOTPOSY_TAG       ("robot_position_y")
#define ICUBHRI_OPC_OBJECT_ROBOTPOSZ_TAG       ("robot_position_z")
#define ICUBHRI_OPC_OBJECT_ROBOTORX_TAG        ("robot_orientation_x")
#define ICUBHRI_OPC_OBJECT_ROBOTORY_TAG        ("robot_orientation_y")
#define ICUBHRI_OPC_OBJECT_ROBOTORZ_TAG        ("robot_orientation_z")
#define ICUBHRI_OPC_OBJECT_RTDIMX_TAG          ("rt_dim_x")
#define ICUBHRI_OPC_OBJECT_RTDIMY_TAG          ("rt_dim_y")
#define ICUBHRI_OPC_OBJECT_RTDIMZ_TAG          ("rt_dim_z")


#define ICUBHRI_OPC_OBJECT_GUI_COLOR_R         ("color_r")
#define ICUBHRI_OPC_OBJECT_GUI_COLOR_G         ("color_g")
#define ICUBHRI_OPC_OBJECT_GUI_COLOR_B         ("color_b")
#define ICUBHRI_OPC_OBJECT_GUI_COLOR_ALPHA     ("color_aplha")

// OPC : Frames
#define ICUBHRI_OPC_FRAME_NAME                 ("frameName")
#define ICUBHRI_OPC_FRAME_MATRIX               ("frameMatrix")
#define ICUBHRI_OPC_FRAME_SCALE                ("frameScale")

// OPC : Spatial relations
#define ICUBHRI_OPC_OBJECT_SPATIAL_CONTAINS    ("contains")
#define ICUBHRI_OPC_OBJECT_SPATIAL_CONTAINED   ("isContained")
#define ICUBHRI_OPC_OBJECT_SPATIAL_INTERSECTS  ("intersects")

// OPC : Kinect Skeleton joints
#define ICUBHRI_OPC_BODY_PART_TYPE_HEAD        ("head")
#define ICUBHRI_OPC_BODY_PART_TYPE_HAND_L      ("handLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_HAND_R      ("handRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_WRIST_L     ("wristLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_WRIST_R     ("wristRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_ELBOW_L     ("elbowLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_ELBOW_R     ("elbowRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_SHOULDER_C  ("shoulderCenter")
#define ICUBHRI_OPC_BODY_PART_TYPE_SHOULDER_L  ("shoulderLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_SHOULDER_R  ("shoulderRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_SPINE       ("spine")
#define ICUBHRI_OPC_BODY_PART_TYPE_HIP_C       ("hipCenter")
#define ICUBHRI_OPC_BODY_PART_TYPE_HIP_L       ("hipLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_HIP_R       ("hipRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_KNEE_L      ("kneeLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_KNEE_R      ("kneeRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_ANKLE_L     ("ankleLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_ANKLE_R     ("ankleRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_FOOT_L      ("footLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_FOOT_R      ("footRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_COLLAR_L    ("collarLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_COLLAR_R    ("collarRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_FT_L        ("fingertipLeft")
#define ICUBHRI_OPC_BODY_PART_TYPE_FT_R        ("fingertipRight")
#define ICUBHRI_OPC_BODY_PART_TYPE_COM         ("CoM")

const std::vector<std::string> ALL_AVAILABLE_BODYPARTS = {
    ICUBHRI_OPC_BODY_PART_TYPE_HEAD,
    ICUBHRI_OPC_BODY_PART_TYPE_HAND_L,
    ICUBHRI_OPC_BODY_PART_TYPE_HAND_R,
    ICUBHRI_OPC_BODY_PART_TYPE_WRIST_L,
    ICUBHRI_OPC_BODY_PART_TYPE_WRIST_R,
    ICUBHRI_OPC_BODY_PART_TYPE_ELBOW_L,
    ICUBHRI_OPC_BODY_PART_TYPE_ELBOW_R,
    ICUBHRI_OPC_BODY_PART_TYPE_SHOULDER_C,
    ICUBHRI_OPC_BODY_PART_TYPE_SHOULDER_L,
    ICUBHRI_OPC_BODY_PART_TYPE_SHOULDER_R,
    ICUBHRI_OPC_BODY_PART_TYPE_SPINE,
    ICUBHRI_OPC_BODY_PART_TYPE_HIP_C,
    ICUBHRI_OPC_BODY_PART_TYPE_HIP_L,
    ICUBHRI_OPC_BODY_PART_TYPE_HIP_R,
    ICUBHRI_OPC_BODY_PART_TYPE_KNEE_L,
    ICUBHRI_OPC_BODY_PART_TYPE_KNEE_R,
    ICUBHRI_OPC_BODY_PART_TYPE_ANKLE_L,
    ICUBHRI_OPC_BODY_PART_TYPE_ANKLE_R,
    ICUBHRI_OPC_BODY_PART_TYPE_FOOT_L,
    ICUBHRI_OPC_BODY_PART_TYPE_FOOT_R,
    ICUBHRI_OPC_BODY_PART_TYPE_COLLAR_L,
    ICUBHRI_OPC_BODY_PART_TYPE_COLLAR_R,
    ICUBHRI_OPC_BODY_PART_TYPE_FT_L,
    ICUBHRI_OPC_BODY_PART_TYPE_FT_R,
    ICUBHRI_OPC_BODY_PART_TYPE_COM
};

//Kinect tags
#define ICUBHRI_KINECT_ALL_INFO              ("all_info")
#define ICUBHRI_KINECT_DEPTH                 ("depth")
#define ICUBHRI_KINECT_DEPTH_PLAYERS         ("depth_players")
#define ICUBHRI_KINECT_DEPTH_RGB             ("depth_rgb")
#define ICUBHRI_KINECT_DEPTH_RGB_PLAYERS     ("depth_rgb_players")
#define ICUBHRI_KINECT_DEPTH_JOINTS          ("depth_joints")
#define ICUBHRI_KINECT_CMD_PING              ("ping")
#define ICUBHRI_KINECT_CMD_ACK               ("ack")
#define ICUBHRI_KINECT_CMD_NACK              ("nack")
#define ICUBHRI_KINECT_CMD_GET3DPOINT        ("get3D")
#define ICUBHRI_KINECT_SEATED_MODE           ("seated")
#define ICUBHRI_KINECT_CLOSEST_PLAYER         -1

/** @} */

#endif
