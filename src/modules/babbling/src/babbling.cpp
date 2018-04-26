/*
 * Copyright (C) 2014 WYSIWYD Consortium, European Commission FP7 Project
 * ICT-612139
 * Authors: Martina Zambelli
 * email:   m.zambelli13@imperial.ac.uk
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

#include "babbling.h"

#include <iostream>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

bool Babbling::configure(yarp::os::ResourceFinder &rf) {
    bool bEveryThingisGood = true;

    string moduleName = rf.check("name", Value("babbling"), "module name (string)").asString();

    robot = rf.check("robot", Value("icub")).asString();
    single_joint = rf.check("single_joint", Value(-1)).asInt();

    Bottle &start_pos = rf.findGroup("start_position");
    Bottle *b_start_commandHead = start_pos.find("head").asList();
    Bottle *b_start_command = start_pos.find("arm").asList();

    start_command_head.resize(3, 0.0);
    if ((b_start_commandHead->isNull()) || (b_start_commandHead->size() < 3)) {
        yWarning("Something is wrong in ini file. Default value is used");
        start_command_head[0] = -20.0;
        start_command_head[1] = -20.0;
        start_command_head[2] = +10.0;
    } else {
        for (int i = 0; i < b_start_commandHead->size(); i++) {
            start_command_head[i] = b_start_commandHead->get(i).asDouble();
            yDebug() << start_command_head[i];
        }
    }

    if ((b_start_command->isNull()) || (b_start_command->size() < 16)) {
        yWarning("Something is wrong in ini file. Default values are used");
        start_command_arm[0] = -45.0; start_command_arm[1] = 35.0; start_command_arm[2] = 0.0;
        start_command_arm[3] = 50.0; start_command_arm[4] = -45.0; start_command_arm[5] = 0.0;
        start_command_arm[6] = 0.0; start_command_arm[7] = 0.0; start_command_arm[8] = 10.0;
        start_command_arm[9] = 0.0; start_command_arm[10] = 0.0; start_command_arm[11] = 0.0;
        start_command_arm[12] = 0.0; start_command_arm[13] = 0.0; start_command_arm[14] = 0.0;
        start_command_arm[15] = 0.0;
    } else {
        for (int i = 0; i < b_start_command->size(); i++)
            start_command_arm[i] = b_start_command->get(i).asDouble();
    }

    Bottle &babbl_par = rf.findGroup("babbling_param");
    freq = babbl_par.check("freq", Value(0.2)).asDouble();
    amp = babbl_par.check("amp", Value(5.0)).asDouble();
    duration = babbl_par.check("duration", Value(20.0)).asDouble();

    setName(moduleName.c_str());

    // Open handler port
    if (!handlerPort.open("/" + getName() + "/rpc")) {
        yError() << getName() << ": Unable to open port " << "/" << getName() << "/rpc";
        bEveryThingisGood = false;
    }

    // Initialize iCub
    yInfo() << "Going to initialise iCub ...";
    while (!init_iCub()) {
        yDebug() << getName() << ": initialising iCub... please wait... ";
    }

    yDebug() << "End configuration...";

    attach(handlerPort);

    return bEveryThingisGood;
}

bool Babbling::interruptModule() {
    handlerPort.interrupt();

    yInfo() << "Bye!";

    return true;
}

bool Babbling::close() {
    yInfo() << "Closing module, please wait ... ";

    leftArmDev.close();
    rightArmDev.close();
    headDev.close();

    handlerPort.interrupt();
    handlerPort.close();

    yInfo() << "Bye!";

    return true;
}

bool Babbling::respond(const Bottle &command, Bottle &reply) {
    string helpMessage = string(getName().c_str()) + " commands are: \n " +
            "babbling arm <left/right>: motor commands sent to all "
            "the arm joints \n " +
            "babbling hand <left/right>: motor commands sent to all "
            "the hand joints \n " +
            "babbling joint <int joint_number> <left/right>: motor "
            "commands sent to joint_number only \n " +
            "help \n " + "quit \n";

    reply.clear();

    if (command.get(0).asString() == "quit") {
        reply.addString("Quitting");
        return false;
    } else if (command.get(0).asString() == "help") {
        yInfo() << helpMessage;
        reply.addString(helpMessage);
    } else if (command.get(0).asString() == "babbling") {
        if (command.get(1).asString() == "arm") {
            single_joint = -1;
            part_babbling = command.get(1).asString();

            if (command.get(2).asString() == "left" || command.get(2).asString() == "right") {
                part = command.get(2).asString() + "_arm";
                yInfo() << "Babbling " + command.get(2).asString() + " arm...";

                if (command.size() >= 4) {
                    yInfo() << "Custom duration = " << command.get(3).asDouble();
                    if (command.get(3).asDouble() >= 0.0) {
                        double newDuration = command.get(3).asDouble();
                        double oldDuration = duration;
                        duration = newDuration;
                        yInfo() << "duration is changed from " << oldDuration << " to " << duration;
                        doBabbling();
                        duration = oldDuration;
                        yInfo() << "Going back to duration from config file: " << duration;
                        reply.addString("ack");
                        return true;
                    } else {
                        yError("Invalid train duration: Must be a double >= 0.0");
                        yWarning("Doing the babbling anyway with default value");
                    }
                }

                doBabbling();
                reply.addString("ack");
                return true;
            } else {
                yError("Invalid babbling part: specify LEFT or RIGHT after 'arm'.");
                reply.addString("nack");
                return false;
            }
        } else if (command.get(1).asString() == "joint") {
            single_joint = command.get(2).asInt();
            if (single_joint < 16 && single_joint >= 0) {
                if (command.get(3).asString() == "left" || command.get(3).asString() == "right") {
                    part = command.get(3).asString() + "_arm";
                    yInfo() << "Babbling joint " << single_joint << " of " << part;

                    // change duration if specified
                    if (command.size() >= 5) {
                        yInfo() << "Custom duration = " << command.get(4).asDouble();
                        if (command.get(4).asDouble() >= 0.0) {
                            double newDuration = command.get(4).asDouble();
                            double oldDuration = duration;
                            duration = newDuration;
                            yInfo() << "duration is changed from " << oldDuration
                                    << " to " << duration;
                            doBabbling();
                            duration = oldDuration;
                            yInfo() << "Going back to duration from config file: "
                                    << duration;
                            reply.addString("ack");
                            return true;
                        } else {
                            yError("Invalid train duration: Must be a double >= 0.0");
                            yWarning("Doing the babbling anyway with default value");
                        }
                    }

                    doBabbling();
                    reply.addString("ack");
                    return true;
                } else {
                    yError("Invalid babbling part: specify LEFT or RIGHT after joint number.");
                    reply.addString("nack");
                    return false;
                }
            } else {
                yError("Invalid joint number.");
                reply.addString("nack");
                return false;
            }
        } else if (command.get(1).asString() == "hand") {
            single_joint = -1;
            part_babbling = command.get(1).asString();

            if (command.get(2).asString() == "left" ||
                    command.get(2).asString() == "right") {
                part = command.get(2).asString() + "_arm";
                yInfo() << "Babbling " + command.get(2).asString() + " hand...";
                doBabbling();
                reply.addString("ack");
                return true;
            } else {
                yError("Invalid babbling part: specify LEFT or RIGHT after 'hand'.");
                reply.addString("nack");
                return false;
            }
        } else {
            yInfo() << "Command not found\n" << helpMessage;
            reply.addString("nack");
            reply.addString("command not found");
            reply.addString(helpMessage);
            return false;
        }
    }

    return true;
}

bool Babbling::updateModule() {
    return true;
}

double Babbling::getPeriod() {
    return 0.1;
}

bool Babbling::doBabbling() {
    bool homeStart = gotoStartPos(); // First go to home position
    if (!homeStart) {
        yError() << "I got lost going home!";
        return false;
    }

    // Change to velocity mode
    for (int i = 0; i < 16; i++) {
        if (part == "right_arm") {
            ictrlRightArm->setControlMode(i, VOCAB_CM_VELOCITY);
        } else if (part == "left_arm") {
            ictrlLeftArm->setControlMode(i, VOCAB_CM_VELOCITY);
        } else {
            yError() << "Don't know which part to move to do babbling.";
            return false;
        }
    }

    double startTime = yarp::os::Time::now();

    yDebug() << "============> babbling with COMMAND will START";
    yInfo() << "AMP " << amp << "FREQ " << freq;

    // now loop and issue the actual babbling commands
    while (Time::now() < startTime + duration) {
        double t = Time::now() - startTime;
        yInfo() << "t = " << t << "/ " << duration;

        babblingCommands(t, single_joint);
    }

    yDebug() << "============> babbling with COMMAND is FINISHED";

    return true;
}

void Babbling::babblingCommands(double &t, int j_idx) {
    double ref_command[16]; // Reference command for the 16 arm joints
    yarp::sig::Vector command; // Command after correction
    yarp::sig::Vector encodersUsed;
    command.resize(16);

    for (unsigned int l = 0; l < command.size(); l++) {
        command[l] = 0;
    }

    for (unsigned int l = 0; l < 16; l++) {
        ref_command[l] = start_command_arm[l] + amp * sin(freq * t * 2 * M_PI);
    }

    if (part == "right_arm" || part == "left_arm") {
        bool okEncArm = false;

        if (part == "right_arm") {
            encodersUsed = encodersRightArm;
            okEncArm = encsRightArm->getEncoders(encodersUsed.data());
        } else {
            encodersUsed = encodersLeftArm;
            okEncArm = encsLeftArm->getEncoders(encodersUsed.data());
        }

        yDebug() << "j_idx: " << j_idx;

        if (j_idx != -1) { // single joint babbling
            if (!okEncArm) {
                yError() << "Error receiving encoders";
                command[j_idx] = 0;
            } else {
                yDebug() << "command before correction = " << ref_command[j_idx];
                yDebug() << "current encoders : " << encodersUsed[j_idx];
                command[j_idx] = amp * sin(freq * t * 2 * M_PI);
                yDebug() << "command after correction = " << command[j_idx];
                if (command[j_idx] > 50) command[j_idx] = 50;
                if (command[j_idx] < -50) command[j_idx] = -50;
                yDebug() << "command after saturation = " << command[j_idx];
            }
        } else {
            if (part_babbling == "arm") {
                if (!okEncArm) {
                    yError() << "Error receiving encoders";
                    for (unsigned int l = 0; l < 7; l++) command[l] = 0;
                } else {
                    for (unsigned int l = 0; l < 7; l++) {
                        command[l] = 10 * (ref_command[l] - encodersUsed[l]);
                        if (command[j_idx] > 20) command[j_idx] = 20;
                        if (command[j_idx] < -20) command[j_idx] = -20;
                    }
                }
            } else if (part_babbling == "hand") {
                if (!okEncArm) {
                    yError() << "Error receiving encoders";
                    for (unsigned int l = 7; l < command.size(); l++) command[l] = 0;
                } else {
                    for (unsigned int l = 7; l < command.size(); l++) {
                        command[l] = 10 * (ref_command[l] - encodersUsed[l]);
                        if (command[j_idx] > 20) command[j_idx] = 20;
                        if (command[j_idx] < -20) command[j_idx] = -20;
                    }
                }
            } else {
                yError("Can't babble the required body part.");
            }
        }

        if (part == "right_arm") {
            velRightArm->velocityMove(command.data());
        } else if (part == "left_arm") {
            velLeftArm->velocityMove(command.data());
        } else {
            yError() << "Don't know which part to move to do babbling.";
        }

        yarp::os::Time::delay(0.05); // This delay is needed!!!

    } else {
        yError() << "Which arm to babble with?";
    }
}

bool Babbling::moveHeadToStartPos() {
    yarp::sig::Vector ang = start_command_head;
    if (part == "right_arm") {
        ang[0] = -ang[0];
    }
    return igaze->lookAtAbsAngles(ang);
}

bool Babbling::gotoStartPos() {
    igaze->stopControl();
    velLeftArm->stop();
    velRightArm->stop();

    yarp::os::Time::delay(2.0);

    moveHeadToStartPos();

    yarp::sig::Vector command; // Command after correction
    command.resize(16);

    /* Move arm to start position */
    if (part == "left_arm" || part == "right_arm") {
        if (part == "left_arm") {
            command = encodersLeftArm;
            for (int i = 0; i < 16; i++) {
                ictrlLeftArm->setControlMode(i, VOCAB_CM_POSITION);
                command[i] = start_command_arm[i];
            }
            posLeftArm->positionMove(command.data());
        } else {
            command = encodersRightArm;
            for (int i = 0; i < 16; i++) {
                ictrlRightArm->setControlMode(i, VOCAB_CM_POSITION);
                command[i] = start_command_arm[i];
            }
            posRightArm->positionMove(command.data());
        }

        bool done_head = false;
        bool done_arm = false;
        while (!done_head || !done_arm) {
            yInfo() << "Wait for position moves to finish";
            igaze->checkMotionDone(&done_head);
            if (part == "left_arm") {
                posLeftArm->checkMotionDone(&done_arm);
            } else {
                posRightArm->checkMotionDone(&done_arm);
            }
            Time::delay(0.04);
        }
        yInfo() << "Done.";

        Time::delay(1.0);
    } else {
        yError() << "Don't know which part to move to start position.";
        return false;
    }

    return true;
}

bool Babbling::init_left_arm() {
    Property option_left;

    string portnameLeftArm = "left_arm";  // part;
    option_left.put("robot", robot.c_str());
    option_left.put("device", "remote_controlboard");
    Value &robotnameLeftArm = option_left.find("robot");

    option_left.put("local", "/babbling/" + robotnameLeftArm.asString() + "/" + portnameLeftArm + "/control");
    option_left.put("remote", "/" + robotnameLeftArm.asString() + "/" + portnameLeftArm);

    yDebug() << "option left arm: " << option_left.toString().c_str();

    if (!leftArmDev.open(option_left)) {
        yError() << "Device not available. Here are the known devices:";
        yError() << yarp::dev::Drivers::factory().toString();
        return false;
    }

    leftArmDev.view(posLeftArm);
    leftArmDev.view(velLeftArm);
    leftArmDev.view(itrqLeftArm);
    leftArmDev.view(encsLeftArm);
    leftArmDev.view(ictrlLeftArm);
    leftArmDev.view(ictrlLimLeftArm);

    double minLimArm[16];
    double maxLimArm[16];
    for (int l = 0; l < 16; l++) {
        ictrlLimLeftArm->getLimits(l, &minLimArm[l], &maxLimArm[l]);
    }

    for (int l = 0; l < 16; l++) {
        yInfo() << "Joint " << l << ": limits = [" << minLimArm[l] << ","
                << maxLimArm[l] << "]. start_commad = " << start_command_arm[l];
    }

    if (posLeftArm == nullptr || encsLeftArm == nullptr || velLeftArm == nullptr ||
            itrqLeftArm == nullptr || ictrlLeftArm == nullptr || encsLeftArm == nullptr) {
        yError() << "Cannot get interface to robot device for left arm";
        leftArmDev.close();
    }

    int nj_left = 0;
    posLeftArm->getAxes(&nj_left);
    encodersLeftArm.resize(nj_left);

    yInfo() << "Wait for arm encoders";
    while (!encsLeftArm->getEncoders(encodersLeftArm.data())) {
        Time::delay(0.1);
        yInfo() << "Wait for arm encoders";
    }

    return true;
}

bool Babbling::init_right_arm() {
    /* Create PolyDriver for right arm */
    Property option_right;

    string portnameRightArm = "right_arm";
    option_right.put("robot", robot.c_str());
    option_right.put("device", "remote_controlboard");
    Value &robotnameRightArm = option_right.find("robot");

    option_right.put("local", "/babbling/" + robotnameRightArm.asString() + "/" + portnameRightArm + "/control");
    option_right.put("remote", "/" + robotnameRightArm.asString() +  "/" + portnameRightArm);

    yDebug() << "option right arm: " << option_right.toString().c_str();

    if (!rightArmDev.open(option_right)) {
        yError() << "Device not available.  Here are the known devices:";
        yError() << yarp::dev::Drivers::factory().toString();
        return false;
    }

    rightArmDev.view(posRightArm);
    rightArmDev.view(velRightArm);
    rightArmDev.view(itrqRightArm);
    rightArmDev.view(encsRightArm);
    rightArmDev.view(ictrlRightArm);
    rightArmDev.view(ictrlLimRightArm);

    double minLimArm[16];
    double maxLimArm[16];
    for (int l = 0; l < 16; l++) {
        ictrlLimRightArm->getLimits(l, &minLimArm[l], &maxLimArm[l]);
    }

    for (int l = 0; l < 16; l++) {
        yInfo() << "Joint " << l << ": limits = [" << minLimArm[l] << ","
                << maxLimArm[l] << "]. start_commad = " << start_command_arm[l];
    }

    if (posRightArm == nullptr || encsRightArm == nullptr || velRightArm == nullptr ||
            itrqRightArm == nullptr || ictrlRightArm == nullptr || encsRightArm == nullptr) {
        yError() << "Cannot get interface to robot device for right arm";
        rightArmDev.close();
    }

    int nj_right = 0;
    posRightArm->getAxes(&nj_right);
    encodersRightArm.resize(nj_right);

    yInfo() << "Wait for arm encoders";
    while (!encsRightArm->getEncoders(encodersRightArm.data())) {
        Time::delay(0.1);
        yInfo() << "Wait for arm encoders";
    }

    return true;
}

bool Babbling::init_iCub() {
    if(!init_left_arm()) {
        return false;
    }
    if(!init_right_arm()) {
        return false;
    }

    yInfo() << "Arms initialized.";

    /* Init. head */
    Property option_head;
    option_head.clear();
    option_head.put("device", "gazecontrollerclient");
    option_head.put("remote", "/iKinGazeCtrl");
    option_head.put("local", "/babbling/gaze");

    if (!headDev.open(option_head)) {
        yError() << "Device not available.  Here are the known devices:";
        yError() << yarp::dev::Drivers::factory().toString();
        return false;
    }

    headDev.view(igaze);
    yInfo() << "Head initialized.";

    yInfo() << "> Initialisation done.";

    return true;
}
