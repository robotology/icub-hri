#include <yarp/os/all.h>
#include "wrdac/subsystems/subSystem_ARE.h"

void icubclient::SubSystem_ARE::appendCartesianTarget(yarp::os::Bottle &b, const yarp::sig::Vector &t)
{
    yarp::os::Bottle &sub=b.addList();
    sub.addString("cartesian");
    for (size_t i=0; i<t.length(); i++)
        sub.addDouble(t[i]);
}

void icubclient::SubSystem_ARE::selectHandCorrectTarget(yarp::os::Bottle &options, yarp::sig::Vector &target, const std::string& objName, const std::string handToUse)
{
    std::string hand="";
    for (int i=0; i<options.size(); i++)
    {
        yarp::os::Value val=options.get(i);
        if (val.isString())
        {
            std::string item=val.asString();
            if ((item=="left") || (item=="right"))
            {
                hand=item;
                break;
            }
        }
    }

    // always choose hand
    if (hand.empty())
    {
        if (handToUse.empty())
        {
            hand=(target[1]>0.0?"right":"left");
            options.addString(hand.c_str());
        }
        else
            hand=handToUse;
    }

    // apply 3D correction
    if (calibPort.getOutputCount()>0)
    {
        yarp::os::Bottle cmd,reply;
        cmd.addString("get_location");
        cmd.addString(hand);
        cmd.addString(objName);
        cmd.addString("iol-"+hand);
        calibPort.write(cmd,reply);
        target[0]=reply.get(1).asDouble();
        target[1]=reply.get(2).asDouble();
        target[2]=reply.get(3).asDouble();

        yarp::os::Bottle opt;
        opt.addString("fixate");
        look(target,opt,objName);
    }

    lastlyUsedHand=hand;
}

bool icubclient::SubSystem_ARE::sendCmd(const yarp::os::Bottle &cmd)
{
    bool ret=false;

    yDebug() << "Send to ARE: " << cmd.toString();
    yarp::os::Bottle bReply;
    if (cmdPort.write(const_cast<yarp::os::Bottle&>(cmd),bReply))
        ret=(bReply.get(0).asVocab()==yarp::os::Vocab::encode("ack"));

    yDebug() << "Reply from ARE: " << bReply.toString();

    return ret;
}

bool icubclient::SubSystem_ARE::connect()
{
    if (!yarp::os::Network::isConnected(calibPort.getName(),"/iolReachingCalibration/rpc")) {
        if (yarp::os::Network::connect(calibPort.getName(),"/iolReachingCalibration/rpc")) {
            yInfo()<<"ARE connected to calibrator";
        } else {
            yWarning()<<"ARE didn't connect to calibrator";
        }
    }

    bool ret=true;
    if(!yarp::os::Network::isConnected(cmdPort.getName(),"/actionsRenderingEngine/cmd:io")) {
        ret&=yarp::os::Network::connect(cmdPort.getName(),"/actionsRenderingEngine/cmd:io");
    }
    if(!yarp::os::Network::isConnected(rpcPort.getName(),"/actionsRenderingEngine/rpc")) {
        ret&=yarp::os::Network::connect(rpcPort.getName(),"/actionsRenderingEngine/rpc");
    }
    if(!yarp::os::Network::isConnected(getPort.getName(),"/actionsRenderingEngine/get:io")) {
        ret&=yarp::os::Network::connect(getPort.getName(),"/actionsRenderingEngine/get:io");
    }

    opc->connect("OPC");

    return ret;
}

icubclient::SubSystem_ARE::SubSystem_ARE(const std::string &masterName) : SubSystem(masterName)
{
    cmdPort.open(("/" + masterName + "/" + SUBSYSTEM_ARE + "/cmd:io").c_str());
    rpcPort.open(("/" + masterName + "/" + SUBSYSTEM_ARE + "/rpc").c_str());
    getPort.open(("/" + masterName + "/" + SUBSYSTEM_ARE + "/get:io").c_str());
    calibPort.open(("/" + masterName + "/" + SUBSYSTEM_ARE + "/calib:io").c_str());
    m_type = SUBSYSTEM_ARE;
    lastlyUsedHand="";

    opc = new OPCClient(m_masterName+"/opc_from_ARE");
}

void icubclient::SubSystem_ARE::Close()
{
    opc->interrupt();
    opc->close();
    delete opc;

    cmdPort.interrupt();
    rpcPort.interrupt();
    getPort.interrupt();
    calibPort.interrupt();

    cmdPort.close();
    rpcPort.close();
    getPort.close();
    calibPort.close();
}

bool icubclient::SubSystem_ARE::getTableHeight(double &height)
{
    yarp::os::Bottle bCmd, bReply;
    bCmd.addVocab(yarp::os::Vocab::encode("get"));
    bCmd.addVocab(yarp::os::Vocab::encode("table"));
    getPort.write(bCmd, bReply);

    yarp::os::Value vHeight = bReply.find("table_height");
    if (vHeight.isNull()) {
        yError("No table height specified in ARE!");
        return false;
    }
    else {
        height = vHeight.asDouble();
        return true;
    }
}

yarp::sig::Vector icubclient::SubSystem_ARE::applySafetyMargins(const yarp::sig::Vector &in)
{
    yarp::sig::Vector out=in;
    out[0]=std::min(out[0],-0.1);

    double height;
    if (getTableHeight(height))
        out[2]=std::max(out[2],height);

    return out;
}

bool icubclient::SubSystem_ARE::home(const std::string &part)
{
    yDebug() << "ARE::home start";
    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("home"));
    bCmd.addString(part.c_str());

    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";
    yDebug() << "ARE::home stop";

    return bReturn;
}

bool icubclient::SubSystem_ARE::take(const std::string &sName, const yarp::os::Bottle &options)
{
    yDebug() << "ARE::take start";

    Entity* e = opc->getEntity(sName, true);
    Object *o;
    if(e) {
        o = dynamic_cast<Object*>(e);
    }
    else {
        yError() << sName << " is not an Entity";
        return false;
    }
    if(!o) {
        yError() << "Could not cast" << e->name() << "to Object";
        return false;
    }

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("take"));

    yarp::sig::Vector target=o->m_ego_position;
    yarp::os::Bottle opt=options;
    selectHandCorrectTarget(opt,target,sName);
    target=applySafetyMargins(target);
    appendCartesianTarget(bCmd,target);
    bCmd.append(opt);

    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";
    yDebug() << "ARE::take stop";

    return bReturn;
}

bool icubclient::SubSystem_ARE::push(const std::string &sName, const yarp::os::Bottle &options)
{
    yDebug() << "ARE::push start";

    Entity* e = opc->getEntity(sName, true);
    Object *o;
    if(e) {
        o = dynamic_cast<Object*>(e);
    }
    else {
        yError() << sName << " is not an Entity";
        return false;
    }
    if(!o) {
        yError() << "Could not cast" << e->name() << "to Object";
        return false;
    }

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("push"));

    yarp::sig::Vector target=o->m_ego_position;
    yarp::os::Bottle opt=options;
    selectHandCorrectTarget(opt,target,sName);
    target=applySafetyMargins(target);
    appendCartesianTarget(bCmd,target);
    bCmd.append(opt);

    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::push stop";
    return bReturn;

}

bool icubclient::SubSystem_ARE::point(const yarp::sig::Vector &targetUnsafe, const yarp::os::Bottle &options, const std::string &sName)
{
    yDebug() << "ARE::pointfar start";
    yarp::sig::Vector target=applySafetyMargins(targetUnsafe);

    yarp::os::Bottle bCmd;
    if(target[0]<-0.31) {
        bCmd.addVocab(yarp::os::Vocab::encode("pfar"));
    } else {
        bCmd.addVocab(yarp::os::Vocab::encode("point"));
    }

    appendCartesianTarget(bCmd,target);
    bCmd.append(options);

    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::pointfar stop";
    return bReturn;
}


/*bool icubclient::SubSystem_ARE::point_old(const std::string &sName, const yarp::os::Bottle &options)
{
    yDebug() << "ARE::point start";

    Entity* e = opc->getEntity(sName, true);
    Object *o;
    if(e) {
        o = dynamic_cast<Object*>(e);
    }
    else {
        yError() << sName << " is not an Entity";
        return false;
    }
    if(!o) {
        yError() << "Could not cast" << e->name() << "to Object";
        return false;
    }

    if(o->m_ego_position[0]<-0.42) {
        yWarning() << "This object is too far for ARE::point. Use ARE::pointfar instead.";
        return pointfar(o->m_ego_position, options, sName);
    }

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("point"));

    yarp::sig::Vector target=o->m_ego_position;
    yarp::os::Bottle opt=options;
    selectHandCorrectTarget(opt,target,sName);
    target=applySafetyMargins(target);
    appendCartesianTarget(bCmd,target);
    bCmd.append(opt);

    bool bReturn = sendCmd(bCmd,true);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::point stop";
    return bReturn;
}*/

bool icubclient::SubSystem_ARE::drop(const yarp::os::Bottle &options)
{
    yDebug() << "ARE::drop start";

    // we don't need d2k correction
    // because the drop takes place
    // on a random location
    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("drop"));
    bCmd.append(options);
    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::drop stop";
    return bReturn;
}

bool icubclient::SubSystem_ARE::dropOn(const yarp::sig::Vector &targetUnsafe, const yarp::os::Bottle &options)
{
    yDebug() << "ARE::dropOn start";

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("drop"));
    bCmd.addString("over");

    yarp::sig::Vector target=targetUnsafe;
    yarp::os::Bottle opt=options;
    selectHandCorrectTarget(opt,target,lastlyUsedHand);
    target=applySafetyMargins(target);
    appendCartesianTarget(bCmd,target);
    bCmd.append(opt);

    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::dropOn stop";
    return bReturn;
}

bool icubclient::SubSystem_ARE::observe(const yarp::os::Bottle &options)
{
    yDebug() << "ARE::observe start";

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("observe"));
    bCmd.append(options);
    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::observe stop";

    return bReturn;
}

bool icubclient::SubSystem_ARE::expect(const yarp::os::Bottle &options)
{
    yDebug() << "ARE::expect start";

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("expect"));
    bCmd.append(options);
    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::expect stop";

    return bReturn;
}

bool icubclient::SubSystem_ARE::give(const yarp::os::Bottle &options)
{
    yDebug() << "ARE::give start";

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("give"));
    bCmd.append(options);
    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::give stop";
    return bReturn;
}

bool icubclient::SubSystem_ARE::waving(const bool sw)
{
    yDebug() << "ARE::waving start";

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("waveing"));
    bCmd.addString(sw ? "on" : "off");
    bool bReturn = rpcPort.asPort().write(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yDebug() << "ARE::waving stop";
    return bReturn;
}

bool icubclient::SubSystem_ARE::look(const yarp::sig::Vector &target, const yarp::os::Bottle &options, const std::string &sName)
{
    yDebug() << "ARE::look start";

    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("look"));
    appendCartesianTarget(bCmd, target);
    bCmd.append(options);
    bool bReturn = sendCmd(bCmd);
    std::string status;
    bReturn ? status = "success" : status = "failed";

    yarp::os::Time::delay(0.1);

    yDebug() << "ARE::look stop";
    return bReturn;
}

bool icubclient::SubSystem_ARE::track(const yarp::sig::Vector &target, const yarp::os::Bottle &options)
{
    // track() is meant for streaming => no point in gating the activity continuously
    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("track"));
    appendCartesianTarget(bCmd, target);
    bCmd.append(options);
    return sendCmd(bCmd);
}

bool icubclient::SubSystem_ARE::impedance(const bool sw)
{
    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("impedance"));
    bCmd.addString(sw ? "on" : "off");
    return rpcPort.asPort().write(bCmd);
}

bool icubclient::SubSystem_ARE::setExecTime(const double execTime)
{
    yarp::os::Bottle bCmd;
    bCmd.addVocab(yarp::os::Vocab::encode("time"));
    bCmd.addDouble(execTime);
    return rpcPort.asPort().write(bCmd);
}

icubclient::SubSystem_ARE::~SubSystem_ARE()
{
    ;
}
