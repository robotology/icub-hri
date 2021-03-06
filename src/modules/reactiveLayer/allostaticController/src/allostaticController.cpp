#include "allostaticController.h"

bool AllostaticController::interruptModule()
{
    yDebug() << "Interrupt rpc port";
    rpc_in_port.interrupt();

    yDebug() << "Interrupt port to homeo rpc";
    to_homeo_rpc.interrupt();
    to_behavior_rpc.interrupt();
    for (auto& outputm_port : outputm_ports)
    {
        outputm_port->interrupt();
    }

    for(auto& outputM_port : outputM_ports)
    {
        outputM_port->interrupt();
    }

    yDebug() << "Interrupt AllostaticDrive ports";
    for(auto& allostaticDrive : allostaticDrives) {
        allostaticDrive.second.interrupt_ports();
    }

    return true;

}

bool AllostaticController::close()
{
    yDebug() << "Closing rpc port";
    rpc_in_port.interrupt();
    rpc_in_port.close();

    yDebug() << "Closing port to homeo rpc";
    to_homeo_rpc.interrupt();
    to_homeo_rpc.close();
    to_behavior_rpc.interrupt();
    to_behavior_rpc.close();

    for (auto& outputm_port : outputm_ports)
    {
        outputm_port->interrupt();
        outputm_port->close();
        delete outputm_port;
    }

    for(auto& outputM_port : outputM_ports)
    {
        outputM_port->interrupt();
        outputM_port->close();
        delete outputM_port;
    }

    yDebug() << "Closing AllostaticDrive ports";
    for(auto& allostaticDrive : allostaticDrives) {
        allostaticDrive.second.close_ports();
    }

    return true;
}

bool AllostaticController::openPorts(string driveName)
{
    bool allGood = true;

    outputm_ports.push_back(new BufferedPort<Bottle>);
    outputM_ports.push_back(new BufferedPort<Bottle>);

    string portName = "/" + moduleName + "/" + driveName;

    // first, min ports
    string pn = portName + "/min:i";

    if (!outputm_ports.back()->open(pn))
    {
        yError() << getName() << ": Unable to open port " << pn;
        allGood = false;
    }
    string targetPortName = "/" + homeo_name + "/" + driveName + "/min:o";
    yarp::os::Time::delay(0.1);
    while(!Network::connect(targetPortName,pn)) {
        yInfo() <<"Setting up homeostatic connections... "<< targetPortName << " " << pn ;
        yarp::os::Time::delay(0.5);
    }

    // now, max ports
    pn = portName + "/max:i";
    yInfo() << "Configuring port " << pn << " ...";
    yarp::os::Time::delay(0.1);
    if (!outputM_ports.back()->open(pn))
    {
        yError() << getName() << ": Unable to open port " << pn;
        allGood = false;
    }
    yarp::os::Time::delay(0.1);
    targetPortName = "/" + homeo_name + "/" + driveName + "/max:o";
    while(!Network::connect(targetPortName, pn))
    {
        yDebug()<<"Setting up homeostatic connections... "<< targetPortName << " " << pn;
        yarp::os::Time::delay(0.5);
    }

    yarp::os::Time::delay(0.1);
    pn = "/" + moduleName + "/toBehaviorManager:o";
    targetPortName = "/BehaviorManager/trigger:i";
    to_behavior_rpc.open(pn);
    while(!Network::connect(pn, targetPortName))
    {
        yDebug()<<"Setting up BehaviorManager connections... "<< pn << " " << targetPortName;
        yarp::os::Time::delay(0.5);
    }
    return allGood;
}

bool AllostaticController::configure(yarp::os::ResourceFinder &rf)
{

    moduleName = rf.check("name",Value("AllostaticController")).asString();
    setName(moduleName.c_str());

    yDebug()<<moduleName<<": finding configuration files...";
    period = rf.check("period",Value(0.5)).asDouble();

    configureAllostatic(rf);

    rpc_in_port.open("/" + moduleName + "/rpc");
    attach(rpc_in_port);

    yInfo()<<"Configuration done.";

    return true;
}

void AllostaticController::configureAllostatic(yarp::os::ResourceFinder &rf)
{
    //The homeostatic module should be running in parallel, independent from this, so the objective of
    //this config would be to have a proper list  and connect to each port

    homeo_name = "homeostasis";
    string homeo_rpc_name = "/" + homeo_name + "/rpc";
    string to_h_rpc_name="/"+moduleName+"/toHomeoRPC:o";
    to_homeo_rpc.open(to_h_rpc_name);

    while(!Network::connect(to_h_rpc_name,homeo_rpc_name))
    {
        yDebug()<<"Trying to connect to homeostasis...";
        yDebug() << "from " << to_h_rpc_name << " to " << homeo_rpc_name;
        yarp::os::Time::delay(0.2);
    }

    yInfo() << "Initializing drives...";
    Bottle grpAllostatic = rf.findGroup("ALLOSTATIC");

    drivesList = *grpAllostatic.find("drives").asList();
    Bottle cmd;

    double priority;
    for (unsigned int d = 0; d<drivesList.size(); d++)
    {
        cmd.clear();
        string driveName = drivesList.get(d).asString();
        yInfo() << ("Initializing drive " + driveName);

        cmd.addString("add");
        cmd.addString("conf");
        Bottle drv;
        drv.clear();
        Bottle aux;
        aux.clear();
        aux.addString("name");
        aux.addString(driveName);
        drv.addList()=aux;
        aux.clear();
        drv.addList()=grpAllostatic;
        cmd.addList()=drv;
        Bottle rply;
        rply.clear();
        rply.get(0).asString();
        to_homeo_rpc.write(cmd,rply);

        AllostaticDrive alloDrive;
        alloDrive.name = driveName;

        Value cmds = grpAllostatic.check((driveName + "-sensation-on"), Value("None"));
        alloDrive.sensationOnCmd = *cmds.asList();
        cmds = grpAllostatic.check((driveName + "-sensation-off"), Value("None"));
        alloDrive.sensationOffCmd = *cmds.asList();

        cmds = grpAllostatic.check((driveName + "-before-trigger"), Value("None"));
        if (!(cmds.isString() && cmds.asString() == "None")) {
            alloDrive.beforeTriggerCmd = *cmds.asList();
        }

        cmds = grpAllostatic.check((driveName + "-after-trigger"), Value("None"));
        if (!(cmds.isString() && cmds.asString() == "None")) {
            alloDrive.afterTriggerCmd = *cmds.asList();
        }        

        alloDrive.homeoPort = &to_homeo_rpc;

        alloDrive.inputSensationPort = new BufferedPort<Bottle>;
        string portName = "/" + moduleName + "/" + driveName + "/sensation:i";
        alloDrive.inputSensationPort->open(portName);

        openPorts(driveName);

        while(!Network::connect("/opcSensation/toHomeo:o", "/homeostasis/fromSensations:i")) {
            yDebug()<<"Connecting " << "/opcSensation/toHomeo:o" << " to " << "/homeostasis/fromSensations:i";
            yarp::os::Time::delay(0.5);
        }


        // set drive priorities. Default to 1.
        priority = grpAllostatic.check((driveName + "-priority"), Value(1.)).asDouble();
        drivePriorities.push_back(priority);

        //Under effects
        string under_port_name = grpAllostatic.check((driveName + "-under-behavior-port"), Value("None")).asString();
        string under_cmd_name = grpAllostatic.check((driveName + "-under-behavior"), Value("None")).asString();

        bool active = false;
        
        if (under_port_name != "None" && under_cmd_name != "None")
        {
            active = true;
            string out_port_name = "/" + moduleName + "/" + driveName + "/under_action:o";
            alloDrive.behaviorUnderPort = new Port();
            alloDrive.behaviorUnderPort->open(out_port_name);
            yDebug() << "trying to connect to" << under_port_name;
            while(!Network::connect(out_port_name,under_port_name))
            {
                
                yarp::os::Time::delay(0.5);
            }
            alloDrive.behaviorUnderCmd = Bottle(under_cmd_name);
        }else{
            yInfo() << "No port name for" << driveName << "under-behavior-port";
        }

        //Over effects
        string over_port_name = grpAllostatic.check((driveName + "-over-behavior-port"), Value("None")).asString();
        string over_cmd_name = grpAllostatic.check((driveName + "-over-behavior"), Value("None")).asString();
        if (over_port_name != "None" && over_cmd_name != "None")
        {
            active=true;
            string out_port_name = "/" + moduleName + "/" + driveName + "/over_action:o";
            alloDrive.behaviorOverPort = new Port();
            alloDrive.behaviorOverPort->open(out_port_name);
            yDebug() << "trying to connect to" << over_port_name;
            while(!Network::connect(out_port_name, over_port_name))
            {
                yarp::os::Time::delay(0.5);
            }
            alloDrive.behaviorOverCmd = Bottle(over_cmd_name);
        } else {
            yInfo() << "No port name for" << driveName << "over-behavior-port";
        }

        alloDrive.active = active;
        allostaticDrives[driveName] = alloDrive;
    }

    if (! Normalize(drivePriorities))
        yDebug() << "Error: Drive priorities sum up to 0.";


    yInfo() << "done.";

}

bool AllostaticController::Normalize(vector<double>& vec) {
    double sum = 0.;
    for (unsigned int i = 0; i<vec.size(); i++)
        sum += vec[i];
    if (sum == 0.)
        return false;
    for (unsigned int i = 0; i<vec.size(); i++)
        vec[i] /= sum;
    return true;
}

bool AllostaticController::updateModule()
{
    double sensationValue;
    for(auto& drive : allostaticDrives) {
        sensationValue = drive.second.inputSensationPort->read()->get(0).asDouble();
        Bottle cmd, reply;
        reply.clear();
        cmd.addString("par");
        cmd.addString(drive.second.name);
        cmd.addString("decaymult");
        cmd.addDouble(sensationValue);
        to_homeo_rpc.write(cmd, reply);
        if (sensationValue) {
            yDebug() << "Sensation ON";
            drive.second.update(SENSATION_ON);
        } else {
            yDebug() << "Sensation OFF";
            drive.second.update(SENSATION_OFF);
        }
    }

    updateAllostatic();

    return true;
}

// Return a DriveOutCZ structure indicating the name and level (UNDER or OVER) of the drive to by humour according to priorities and homeostatis levels
// name is "None" if no drive to be solved
DriveOutCZ AllostaticController::chooseDrive() {

    DriveOutCZ result;
    bool inCZ;
    int numOutCz = 0;
    double random, cum;
    vector<double> outOfCzPriorities(drivePriorities);
    vector<string> names;
    vector<double> min_diff; 
    vector<double> max_diff; 

    for (unsigned int i=0;i<drivesList.size();i++) {
        names.push_back(drivesList.get(i).asString());
        min_diff.push_back(outputm_ports[i]->read()->get(0).asDouble());
        max_diff.push_back(outputM_ports[i]->read()->get(0).asDouble());
        inCZ = min_diff.back() <= 0 && max_diff.back() <= 0;
        if (inCZ) {
            outOfCzPriorities[i] = 0.;
        }
        else {
            numOutCz ++;
        }
    }
    if (! numOutCz) {
        result.name = "None";
        result.level = UNDEFINED;
        return result;
    }
    if ( ! Normalize(outOfCzPriorities)) {
        result.name = "None";
        result.level = UNDEFINED;
        return result;
    }
    random = Rand::scalar();
    cum = outOfCzPriorities[0];
    int idx = 0;
    while (cum < random) {
        cum += outOfCzPriorities[idx + 1];
        idx++;
    }
    result.name = names[idx];
    if (min_diff[idx] > 0)
        result.level = UNDER;
    else if (max_diff[idx] > 0)
        result.level = OVER;
    else
        result.level = UNDEFINED;
    return result;
}


bool AllostaticController::updateAllostatic()
{
    DriveOutCZ activeDrive = chooseDrive();

    yInfo() << "Drive " + activeDrive.name + " chosen";

    if (activeDrive.name == "None") {
        yInfo() << "No drive out of CZ." ;
        return true;
    }
    else
    {
        yInfo() << "Drive " + activeDrive.name + " out of CZ." ;
    }

    if (allostaticDrives[activeDrive.name].active) {
        yInfo() << "Trigerring " + activeDrive.name;
        allostaticDrives[activeDrive.name].triggerBehavior(activeDrive.level);
    }
    else {
        yInfo() << "Drive " + activeDrive.name + " is not active";
    }

    return true;
}


bool AllostaticController::respond(const Bottle& cmd, Bottle& reply)
{
    yInfo() << "RPC received in allostaticController";
    yDebug() << cmd.toString();
    
    reply.clear();

    if (cmd.get(0).asString() == "help" )
    {   string help = "\n";
        help += " [manual on/off]  : Turns on/off manual mode (for manual control of drives) \n";
        reply.addString(help);
    }
    else if (cmd.get(0).asString() == "manual") {
        if (cmd.get(1).asString() == "on") {
            for(auto& allostaticDrive : allostaticDrives) {
                allostaticDrive.second.manualMode = true;
            }
            yInfo() << "Manual mode turns on";
            reply.addString("ack");
        } else if (cmd.get(1).asString() == "off") {
            for(auto& allostaticDrive : allostaticDrives) {
                allostaticDrive.second.manualMode = false;
            }           
            yInfo() << "Manual mode turns off";
            reply.addString("ack");
        }
        Bottle bmCmd, bmReply;
        bmCmd.addString("manual");  
        bmCmd.addString(cmd.get(1).asString());  // on or off
        to_behavior_rpc.write(bmCmd, bmReply);
    } else {
        reply.addString("nack");
        reply.addString("Unknown rpc command");
    }
    return true;
}

bool AllostaticDrive::interrupt_ports() {
    if (behaviorUnderPort!=nullptr) {
        behaviorUnderPort->interrupt();
    }
    if (behaviorOverPort!=nullptr) {
        behaviorOverPort->interrupt();
    }
    if (inputSensationPort!=nullptr) {
        inputSensationPort->interrupt();
    }
    return true;
}

bool AllostaticDrive::close_ports() {
    if (behaviorUnderPort!=nullptr) {
        behaviorUnderPort->interrupt();
        behaviorUnderPort->close();
        delete behaviorUnderPort;
        behaviorUnderPort=nullptr;
    }
    if (behaviorOverPort!=nullptr) {
        behaviorOverPort->interrupt();
        behaviorOverPort->close();
        delete behaviorOverPort;
        behaviorOverPort=nullptr;
    }
    if(inputSensationPort!=nullptr) {
        inputSensationPort->interrupt();
        inputSensationPort->close();
        delete inputSensationPort;
        inputSensationPort=nullptr;
    }
    return true;
}

Bottle AllostaticDrive::update(DriveUpdateMode mode)
{
    Bottle cmds;
    switch (mode) {
    case SENSATION_ON:
        cmds = sensationOnCmd;
        break;
    case SENSATION_OFF:
        cmds = sensationOffCmd;
        break;
    default:
        yDebug() << "Update mode not implemented";
        yDebug() << to_string(mode);
        break;
    }
    Bottle rplies;
    for (unsigned int i=0; i<cmds.size(); i++){
        Bottle rply;
        Bottle cmd = *cmds.get(i).asList();
        if (! manualMode) {
            homeoPort->write(cmd,rply);
        }
        rplies.addList() = rply;
    }
    return rplies;
}

void AllostaticDrive::triggerBehavior(OutCZ mode)
{
    Bottle cmd, rply, rplies;
    // before trigger command
    if (! beforeTriggerCmd.isNull() && !manualMode) {
        cmd.clear();
        rply.clear();
        rplies.clear();
        for (unsigned int i=0; i<beforeTriggerCmd.size(); i++){
            rply.clear();
            Bottle cmd = *beforeTriggerCmd.get(i).asList();
            yDebug() << cmd.toString();
            homeoPort->write(cmd,rply);
            rplies.addList() = rply;
        }
    }

    Port* port = nullptr;
    switch (mode) {
    case UNDER:
        cmd = behaviorUnderCmd;
        port = behaviorUnderPort;
        break;
    case OVER:
        cmd = behaviorOverCmd;
        port = behaviorOverPort;
        break;
    default:
        yWarning() << "Trigger mode not implemented";
        yWarning() << to_string(mode);
        return;
    }

    yInfo() << "Drive " + name + " to be triggered via " << port->getName();
    port->write(cmd, rply);

    // after trigger command
    if ( ! afterTriggerCmd.isNull() && ! manualMode) {
        cmd.clear();
        rply.clear();
        rplies.clear();
        for (unsigned int i=0; i<afterTriggerCmd.size(); i++){
            rply.clear();
            Bottle cmd = *afterTriggerCmd.get(i).asList();
            yDebug() << cmd.toString();
            homeoPort->write(cmd,rply);
            rplies.addList() = rply;
            yDebug() << "triggerBehavior completed.";
        }
    }
}
