#ifndef __CVZ_ICVZ_H__
#define __CVZ_ICVZ_H__

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "IModality.h"
#include "helpers.h"

class IConvergenceZone: public yarp::os::RFModule
{
private:
	yarp::os::Port rpcPort;
	bool respond(const yarp::os::Bottle &cmd, yarp::os::Bottle &reply)
	{
		std::string kWord1 = cmd.get(0).asString();
		if (kWord1 == "start")
		{
			isPaused = false;
			reply.addString("ACK");
			reply.addString("Started");
		}
		else if (kWord1 == "stop")
		{
			isPaused = true;
			reply.addString("ACK");
			reply.addString("Paused");
		}
		else if (!interpretRpcCommand(cmd,reply))
		{
			reply.addString("NACK");
			reply.addString("Cmd unknown");
		}
		return true;
	}

protected:
	int cyclesElapsed;
	double period;
	bool isPaused;

	virtual bool interpretRpcCommand(const yarp::os::Bottle &cmd, yarp::os::Bottle &reply)
	{
		return false;
	}

public:    

	std::map<std::string, IModality*> modalitiesBottomUp;
	std::map<std::string, IModality*> modalitiesTopDown;

	bool configure(yarp::os::ResourceFinder &rf)
	{
		std::string name = rf.check("name",yarp::os::Value("defaultCvz")).asString();
		period = rf.check("period",yarp::os::Value(0.01)).asDouble();
		setName(name.c_str());

		int modalityCount = 0;

		yarp::os::Bottle bMod;bMod.addDouble(0);
		bool reachedLastModality = false;
		while (!reachedLastModality)
		{		
			std::string ss = "modality_";
			ss += int2str(modalityCount);

			bMod = rf.findGroup(ss.c_str());
			if (bMod.size() == 0)
			{
				reachedLastModality = true;
				break;
			}

			std::cout<<name<<" is configuring "<<ss<<std::endl;
			//Get the generic parameters (name, size, minBounds, maxBounds, isTopDown)
			std::string modName = bMod.find("name").asString();
			int modSize = bMod.find("size").asInt();
			std::vector<double> minBounds;
			if (bMod.check("minBounds"))
			{
				yarp::os::Bottle* bMask = bMod.find("minBounds").asList();
				for(int i=0; i<bMask->size(); i++)
					minBounds.push_back(bMask->get(i).asDouble());
			}
			else
				minBounds.resize(modSize,0.0);

			std::vector<double> maxBounds;
			if (bMod.check("maxBounds"))
			{
				yarp::os::Bottle* bMask = bMod.find("maxBounds").asList();
				for(int i=0; i<bMask->size(); i++)
					maxBounds.push_back(bMask->get(i).asDouble());
			}
			else
				maxBounds.resize(modSize,1.0);

			bool isTopDown = bMod.check("isTopDown");

			//Get the type and any additional parameters
			std::string modType = bMod.find("type").asString();
			IModality* mod = NULL;
			if(modType == "yarpVector")
			{
				std::vector<bool> mask;
				if (bMod.check("mask"))
				{
					yarp::os::Bottle* bMask = bMod.find("mask").asList();
					for(int i=0; i<bMask->size(); i++)
						mask.push_back(bMask->get(i).asDouble());
				}
                bool isBlocking = bMod.check("isBlocking");
				std::string modPortPrefix = "/";
				modPortPrefix += getName() + "/";
				modPortPrefix += modName;
                mod = new ModalityBufferedPort<yarp::os::Bottle>(modPortPrefix, modSize, minBounds, maxBounds, mask, isBlocking);

				std::string autoConnect = bMod.check("autoconnect", yarp::os::Value("")).asString();
				if (autoConnect != "")
					((ModalityBufferedPort<yarp::os::Bottle>*)mod)->ConnectInput(autoConnect);
			}
			else if (modType == "yarpImageFloat")
			{
				std::vector<bool> mask;
				if (rf.check("mask"))
				{
					yarp::os::Bottle* bMask = rf.find("mask").asList();
					for(int i=0; i<bMask->size(); i++)
						mask.push_back(bMask->get(i).asDouble());
				}
                bool isBlocking = bMod.check("isBlocking");
				std::string modPortPrefix = "/";
				modPortPrefix += getName() + "/";
				modPortPrefix += modName;
                mod = new ModalityBufferedPort<yarp::sig::ImageOf<yarp::sig::PixelFloat> >(modPortPrefix, modSize, minBounds, maxBounds, mask, isBlocking);

				std::string autoConnect = bMod.check("autoconnect", yarp::os::Value("")).asString();
				if (autoConnect != "")
					((ModalityBufferedPort<yarp::sig::ImageOf<yarp::sig::PixelFloat> >*)mod)->ConnectInput(autoConnect);
			}
			else if (modType == "yarpImageRgb")
			{
				std::vector<bool> mask;
				if (rf.check("mask"))
				{
					yarp::os::Bottle* bMask = rf.find("mask").asList();
					for (int i = 0; i<bMask->size(); i++)
						mask.push_back(bMask->get(i).asDouble());
				}
                bool isBlocking = bMod.check("isBlocking");
				std::string modPortPrefix = "/";
				modPortPrefix += getName() + "/";
				modPortPrefix += modName;
                mod = new ModalityBufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >(modPortPrefix, modSize, minBounds, maxBounds, mask, isBlocking);

				std::string autoConnect = bMod.check("autoconnect", yarp::os::Value("")).asString();
				if (autoConnect != "")
					((ModalityBufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >*)mod)->ConnectInput(autoConnect);
			}
			else
			{
				std::cout<<"Warning, this modality type does not exist. Discarded."<<std::endl;
			}

			if (mod!=NULL)
			{
				if (isTopDown)
					modalitiesTopDown[mod->Name()] = mod;
				else
					modalitiesBottomUp[mod->Name()] = mod;
			}

			modalityCount++;
			ss.clear();
		}

		isPaused = false;
		std::string rpcName = "/";
		rpcName += getName() + "/rpc";
		rpcPort.open(rpcName.c_str());
		attach(rpcPort);

		std::cout<<std::endl<<"Modalities added. Starting the CVZ process with "<<period<<"s period"<<std::endl;
		cyclesElapsed = 0;
		return true;
	}

    bool interruptModule()
    {        
		for(std::map<std::string, IModality*>::iterator it = modalitiesBottomUp.begin(); it != modalitiesBottomUp.end(); it++)
		{
			it->second->Interrupt();
		}		
		for(std::map<std::string, IModality*>::iterator it = modalitiesTopDown.begin(); it != modalitiesTopDown.end(); it++)
		{
			it->second->Interrupt();
		}
        return true;
    }

    virtual bool close()
    {
        for(std::map<std::string, IModality*>::iterator it = modalitiesBottomUp.begin(); it != modalitiesBottomUp.end(); it++)
		{
			it->second->Close();
		}		
		for(std::map<std::string, IModality*>::iterator it = modalitiesTopDown.begin(); it != modalitiesTopDown.end(); it++)
		{
			it->second->Close();
		}

        return true;
    }

    double getPeriod()
    {
        return period;
    }

    bool updateModule()
	{
		//Read the modalities
        for(std::map<std::string, IModality*>::iterator it = modalitiesBottomUp.begin(); it != modalitiesBottomUp.end(); it++)
		{
			it->second->Input();
		}		
		for(std::map<std::string, IModality*>::iterator it = modalitiesTopDown.begin(); it != modalitiesTopDown.end(); it++)
		{
			it->second->Input();
		}

		//Do some computation
		ComputePrediction();

		//Write the modalities
        for(std::map<std::string, IModality*>::iterator it = modalitiesBottomUp.begin(); it != modalitiesBottomUp.end(); it++)
		{
			it->second->Output();
		}		
		for(std::map<std::string, IModality*>::iterator it = modalitiesTopDown.begin(); it != modalitiesTopDown.end(); it++)
		{
			it->second->Output();
		}
		std::cout << getName() << "\t t=" << cyclesElapsed << std::endl;
		cyclesElapsed++;
		return true;
	}

	virtual void ComputePrediction()
	{
		std::cout<<"Warning: You are using a dummy class! The base class of CVZ just send back the input."<<std::endl;

		//Copy input to prediction
        for(std::map<std::string, IModality*>::iterator it = modalitiesBottomUp.begin(); it != modalitiesBottomUp.end(); it++)
		{
			it->second->SetValuePrediction(it->second->GetValueReal());
		}		
		for(std::map<std::string, IModality*>::iterator it = modalitiesTopDown.begin(); it != modalitiesTopDown.end(); it++)
		{
			it->second->SetValuePrediction(it->second->GetValueReal());
		}
	}


};

#endif