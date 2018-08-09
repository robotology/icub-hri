#include "test.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace icubclient;

void TestSensation::configure()
{
    in.open("/Sensation/test/in");
    out.open("/Sensation/test/out");
    cout<<"Configuration done."<<endl;
}

void TestSensation::publish()
{
    Bottle *res = in.read(false);
    if (res != NULL) {
        on = (res->get(0).asString() == "ON");
        cout << "TEST is "<< on << endl;
    }

    yarp::os::Bottle &tosend = out.prepare();
    tosend.clear();
    tosend.addInt(int(on));
    out.write();
}

