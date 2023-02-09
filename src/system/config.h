#pragma once

#include <map>
#include <string>
#include <set>
#include <vector>

// TODO: add string vector storage

namespace Config
{
    void ParseCommandLine(int argc, char* argv[]);

    void ParseFile(std::string filename);
    void SaveCurrentConfigToFile(std::string filename);

    bool GetBool(std::string key, bool defaultValue = false);
    void SetBool(std::string key, bool value);

    int GetInt(std::string key, int defaultValue = 0);
    void SetInt(std::string key, int value);

    bool StringExists(std::string key);
    const std::string GetString(std::string key);
	void SetString(std::string key, std::string value);

    bool StringVectorExists(std::string key);
    const std::vector<std::string> GetStringVector(std::string key);

    void Initialise();
    void Dump();

    bool FindTypeKeyValue(const std::string& line, std::string& type, std::string& key, std::string& value);
}
