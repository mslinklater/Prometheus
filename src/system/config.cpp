#include "config.h"
#include "log.h"
#include <string.h>

static Config* instance = nullptr;

Config* Config::Instance()
{
    if (instance == nullptr)
    {
        instance = new (Config);

        instance->Initialise();
    }

    return instance;
}

void Config::ParseCommandLine(int argc, char* argv[])
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
    std::string configPath("../config.txt");

    // load from file

    FILE* hFile = fopen(configPath.c_str(), "r");

    if (hFile == nullptr)
    {
        LOGERRORF("Config::Unable to load config file '%s'", configPath.c_str());
        // TODO add exception
    }
    else
    {
        char* pFileBuffer;
        fseek(hFile, 0, SEEK_END);
        long fileSize = ftell(hFile);
        pFileBuffer = (char*)malloc(fileSize);
        fseek(hFile, 0, SEEK_SET);
        long bytesRead = fread(pFileBuffer, 1, fileSize, hFile);

        // now parse
        const char* pRead = pFileBuffer;
        const char* pLookahead = pRead;

        while (pRead < pFileBuffer + fileSize)
        {
            while ((pLookahead < pFileBuffer + fileSize) && (*pLookahead != 0x0a))
            {
                // parse line
                pLookahead++;
            }
            // parse the line
            std::string line = pRead;

            std::string type;
            std::string key;
            std::string value;

            FindTypeKeyValue(line, type, key, value);

            if (type == "bool")
            {
                boolSettings.emplace(key, (value == "true") ? true : false);
            }
            else if (type == "int")
            {
                intSettings.emplace(key, atoi(value.c_str()));
            }
            else if (type == "string")
            {
                stringSettings.emplace(key, value);
            }
            else if (type == "category")
            {
                LOGINFO("[Config]Found category");
                // TODO: Add category support
            }
            else
            {
                LOGFATALF("Config:Unknown line %s", pRead);
            }

            pRead = ++pLookahead;
        }

        free(pFileBuffer);
        fclose(hFile);
    }
}

bool Config::FindTypeKeyValue(const std::string& line, std::string& type, std::string& key, std::string& value)
{
    // first lets try to detect malformed setting lines

    size_t openPos = line.find("[");
    size_t closePos = line.find("]");
    size_t equalPos = line.find("=");
    size_t newlinePos = line.find("\n");

    if ((openPos == std::string::npos) || (closePos == std::string::npos) || (equalPos == std::string::npos))
        return false;

    if ((openPos > closePos) || (closePos > equalPos))
        return false;

    type = line.substr(openPos + 1, closePos - openPos - 1);
    key = line.substr(closePos + 1, equalPos - closePos - 1);
    value = line.substr(equalPos + 1, newlinePos - equalPos - 1);

    LOGINFOF("Found %s %s %s", type.c_str(), key.c_str(), value.c_str());

    return true;
}

bool Config::GetBool(std::string key)
{
    if (boolSettings.find(key) != boolSettings.end())
    {
        return boolSettings[key];
    }
    return false;
}

std::string Config::GetString(std::string key)
{
    if (stringSettings.find(key) != stringSettings.end())
    {
        return stringSettings[key];
    }
    return "INVALID";
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
