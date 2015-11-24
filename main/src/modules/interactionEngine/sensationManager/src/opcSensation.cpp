#include <algorithm>    // std::random_shuffle
#include "opcSensation.h"

void UnknownObjects::configure()
{
    moduleName = "opcSensation";
    bool isRFVerbose = false;
    iCub = new ICubClient(moduleName,"sensation","client.ini",isRFVerbose);
    iCub->opc->isVerbose = false;
    char rep = 'n';
    yDebug() << "HERE" ;  
    while (rep!='y'&&!iCub->connect())
    {
        cout<<"iCubClient : Some dependencies are not running..."<<endl;
        //cout<<"Continue? y,n"<<endl;
        //cin>>rep;
        break; //to debug
        Time::delay(1.0);
    }

    confusion_port_name = "/" + moduleName + "/confusion:o"; //This goes to homeostasis
    confusion_port.open(confusion_port_name);

    unknown_obj_port_name = "/" + moduleName + "/unknown_obj:o"; //this goes to behaviors
    unknown_obj_port.open(unknown_obj_port_name);

    // Rand::init();

    cout<<"Configuration done."<<endl;

}

void UnknownObjects::publish()
{
    Bottle res = handleTagging();

    yarp::os::Bottle &confus = confusion_port.prepare();
    confus.clear();
    confus.addInt(int(res.get(0).asInt()));
    confusion_port.write();

    yarp::os::Bottle &unkn = unknown_obj_port.prepare();
    unkn.clear();
    unkn.append(*res.get(1).asList());
    unknown_obj_port.write();

}

Bottle UnknownObjects::handleTagging()
{
    iCub->opc->checkout();
    list<Entity*> lEntities = iCub->opc->EntitiesCache();

    bool unknown_obj = false;
    Bottle u_objects;
    Bottle ob;

    for (list<Entity*>::iterator itEnt = lEntities.begin(); itEnt != lEntities.end(); itEnt++)
    {
        string sName = (*itEnt)->name();
        string sNameCut = sName;
        string original_name = sName;
        string delimiter = "_";
        size_t pos = 0;
        string token;
        if ((pos = sName.find(delimiter)) != string::npos) {
            token = sName.substr(0, pos);
            sName.erase(0, pos + delimiter.length());
            sNameCut = token;
        }
        // check if label is known

        if (sNameCut == "unknown") {
            if ((*itEnt)->entity_type() == "object")//|| (*itEnt)->entity_type() == "agent" || (*itEnt)->entity_type() == "rtobject")
            {
                // yInfo() << "I found an unknown entity: " << sName;
                Object* o = dynamic_cast<Object*>(*itEnt);
                if(o && o->m_present) {
                    unknown_obj = true;
                    //Output is a list of objects entity + objects name [the arguments for tagging]
                    // o->m_saliency = unknownObjectSaliency;
                    ob.clear();
                    ob.addString((*itEnt)->entity_type());
                    ob.addString(original_name);
                    u_objects.addList()=ob;
                    //Could also send saliency
                }
            } else if((*itEnt)->entity_type() == "bodypart") {
                ;//unknown_obj = true;
            }
        } else {
            if ((*itEnt)->entity_type() == "bodypart" && (dynamic_cast<Bodypart*>(*itEnt)->m_tactile_number == -1 || dynamic_cast<Bodypart*>(*itEnt)->m_kinStruct_instance == -1))
                {}//unknown_obj = true;
        }
    }
    //if no unknown object was found, return false

    Bottle out;
    out.addInt(int(unknown_obj));
    out.addList()=u_objects;
    // cout << out.toString()<<endl;
    return out;
}



