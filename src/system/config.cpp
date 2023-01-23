#include "config.h"
#include "log.h"
#include <string.h>
#include <sstream>
#include <unistd.h>

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
        LOGFATALF("Config::Unable to open config file '%s' cwd:%s", configPath.c_str(), get_current_dir_name());
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
            std::string line(pRead, pLookahead - pRead);

            std::string type;
            std::string key;
            std::string value;

            if(FindTypeKeyValue(line, type, key, value))
            {
                std::string fullKey = currentCategory + std::string(".") + key;

                if (type == "bool")
                {
                    boolSettings.emplace(fullKey, (value == "true") ? true : false);
                }
                else if (type == "int")
                {
                    intSettings.emplace(fullKey, atoi(value.c_str()));
                }
                else if (type == "string")
                {
                    stringSettings.emplace(fullKey, value);
                }
                else if (type == "stringvector")
                {
					std::vector<std::string> empty;
					stringVectorSettings.emplace(fullKey, empty);
					// split the value by the delimeter, in this case ','
					std::stringstream valuestream(value);
					std::string parsed;
					while(getline(valuestream, parsed, ','))
					{
						stringVectorSettings[fullKey].push_back(parsed);
					}
                }
                else if (type == "category")
                {
                    currentCategory = value;
                    if(categories.find(currentCategory) == categories.end())
                    {
                        categories.emplace(currentCategory);
                    }
                }
                else
                {
                    LOGFATALF("Config:Unknown line %s", pRead);
                }
            }

            pRead = ++pLookahead;
        }

        free(pFileBuffer);
        fclose(hFile);
    }

    if(GetBool("config.dumpafterinit", false))
    {
        Dump();
    }
    return;
}

void Config::Dump()
{
    LOGINFO("====================");
    LOGINFO("Config::Dumping bool");
    LOGINFO("====================");
	for (auto bools = boolSettings.begin(); bools != boolSettings.end(); ++bools)
	{
		LOGINFOF("%s = %s", bools->first.c_str(), bools->second ? "true" : "false");
	}
}

bool Config::FindTypeKeyValue(const std::string& line, std::string& type, std::string& key, std::string& value)
{
    // first lets try to detect malformed setting lines

    size_t openPos = line.find("[");
    size_t closePos = line.find("]");
    size_t equalPos = line.find("=");
    size_t endlinePos = line.size();

    if ((openPos == std::string::npos) || (closePos == std::string::npos))
        return false;

    if ((openPos > closePos) || (closePos > equalPos))
        return false;

    if(equalPos == std::string::npos)
    {
        // not a line with an equals
        type = line.substr(openPos + 1, closePos - openPos - 1);
        value = line.substr(closePos + 1, endlinePos - closePos - 1);
    }
    else
    {
        type = line.substr(openPos + 1, closePos - openPos - 1);
        key = line.substr(closePos + 1, equalPos - closePos - 1);
        value = line.substr(equalPos + 1, endlinePos - equalPos - 1);
    }

    return true;
}

bool Config::GetBool(std::string key, bool defaultValue)
{
    if (boolSettings.find(key) != boolSettings.end())
    {
        return boolSettings[key];
    }
    return defaultValue;
}

void Config::SetBool(std::string key, bool value)
{
    LOGINFOF("Config:SetBool %s %s", key.c_str(), value ? "true" : "false");
    boolSettings[key] = value;
}

int Config::GetInt(std::string key, int defaultValue)
{
    if (intSettings.find(key) != intSettings.end())
    {
        return intSettings[key];
    }
    return defaultValue;
}

void Config::SetInt(std::string key, int value)
{
    LOGINFOF("Config:SetInt %s %d", key.c_str(), value);
    intSettings[key] = value;
}

bool Config::StringExists(std::string key)
{
    return stringSettings.find(key) != stringSettings.end();
}

const std::string Config::GetString(std::string key)
{
    if (stringSettings.find(key) != stringSettings.end())
    {
        return stringSettings[key];
    }
    return "INVALID";
}

bool Config::StringVectorExists(std::string key)
{
    return stringVectorSettings.find(key) != stringVectorSettings.end();
}

const std::vector<std::string> Config::GetStringVector(std::string key)
{
    if (stringVectorSettings.find(key) != stringVectorSettings.end())
    {
        return stringVectorSettings[key];
    }

	// not sure about this... hacky to return a const reference to a static ?
	std::vector<std::string> temp;
    return temp;
}

void Config::ParseFile(std::string filename)
{
    LOGINFOF("Config:ParseFile %s", filename.c_str());
}

void Config::SaveCurrentConfigToFile(std::string filename)
{
    LOGINFOF("Config:SaveCurrentConfigToFile %s", filename.c_str());
}
