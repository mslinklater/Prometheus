#include "config.h"
#include "log.h"
#include <string.h>

static Config *instance = nullptr;

Config *Config::Instance()
{
    if (instance == nullptr)
    {
        instance = new (Config);

        instance->Initialise();
    }

    return instance;
}

void Config::ParseCommandLine(int argc, char *argv[])
{
    LOGINFO("Config:ParseCommandLine");
    // find all the booleans
    for (int i = 0; i < argc; i++)
    {
        if ((argv[i][0] == '-') && (argv[i][1] != '-'))
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
    if (boolSettings.find(key) != boolSettings.end())
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

bool Config::StringExists(std::string key)
{
    return stringSettings.find(key) != stringSettings.end();
}

void Config::ParseFile(std::string filename)
{
    LOGINFOF("Config:ParseFile %s", filename.c_str());
}

void Config::SaveCurrentConfigToFile(std::string filename)
{
    LOGINFOF("Config:SaveCurrentConfigToFile %s", filename.c_str());
}
