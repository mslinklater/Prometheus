#pragma once

#include <map>
#include <string>

class Config
{
  public:
    static Config *Instance();

    void ParseCommandLine(int argc, char *argv[]);
    void ParseFile(std::string filename);
    void SaveCurrentConfigToFile(std::string filename);

    bool GetBool(std::string key);
    void SetBool(std::string key, bool value);

    bool StringExists(std::string key);
    std::string GetString(std::string key);

  private:
    void Initialise();

    std::map<std::string, bool> boolSettings;
    std::map<std::string, std::string> stringSettings;
};
