#include "recognitionOrder.h"

using namespace std;
using namespace yarp::os;

void RecognitionOrder::configure() {
    from_sensation_port_name = "None";

    homeoPort = "/homeostasis/rpc";

    port_to_homeo_name = "/"+behaviorName+"/toHomeo:o";
    port_to_homeo.open(port_to_homeo_name);

    manual = true;
}

void RecognitionOrder::run(const Bottle &/*args*/) {
    yInfo() << "recognitionOrder::run";

    if (!Network::isConnected(port_to_homeo_name,homeoPort)){
        if (!Network::connect(port_to_homeo_name,homeoPort)){
            yWarning()<<"Port to Homeostasis not available. Could not freeze the drives...";
        }
    }
    if (Network::isConnected(port_to_homeo_name,homeoPort)){
        yInfo()<<"freezing drives";
        Bottle cmd;
        Bottle rply;
        cmd.addString("freeze");
        cmd.addString("all");

        port_to_homeo.write(cmd, rply);
    }

    yDebug() << "send rpc to SAM";
    
    if (iCub->getSAMClient() && iCub->getSAMClient()->askXLabel("action"))
    {
        toSay = iCub->getSAMClient()->classification;
    } else {
        toSay = "Sorry. I did not recognise that action";
    }
    iCub->lookAtPartner();
    iCub->say(toSay);
    iCub->home();
    yInfo() << "Recognising ends";

    if (!manual && Network::isConnected(port_to_homeo_name, homeoPort)){
        yInfo()<<"unfreezing drives";
        Bottle cmd;
        Bottle rply;
        cmd.addString("unfreeze");
        cmd.addString("all");
        port_to_homeo.write(cmd, rply);
    }
}
