#include "config.h"
#include <string.h>
#include "log.h"

static Config* instance = nullptr;

Config* Config::Instance()
{
	if(instance == nullptr)
	{
		instance = new(Config);

		instance->Initialise();
	}

	return instance;
}

void Config::ParseCommandLine(int argc, char *argv[])
{
	// find all the booleans
	for(int i=0 ; i<argc ; i++)
	{
		if((argv[i][0] == '-') && (argv[i][1] != '-'))
		{
			// found boolean
			std::string key = &(argv[i][1]);
			SetBool(key, true);
		}
	}
}

void Config::Initialise()
{
	// load from JSON file

	// load boolean array
	
}

bool Config::GetBool(std::string key)
{
	if(boolSettings.find(key) != boolSettings.end())
	{
		return boolSettings[key];
	}
	return false;
}

void Config::SetBool(std::string key, bool value)
{
	LOGINFOF("Config:SetBool %s %s", key.c_str(), value ? "true" : "false");
	boolSettings[key] = value;
}
