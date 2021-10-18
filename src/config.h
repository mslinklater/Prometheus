#pragma once

#include <string>
#include <map>

class Config
{
public:
	static Config* Instance();

	void ParseCommandLine(int argc, char *argv[]);

	bool GetBool(std::string key);
	void SetBool(std::string key, bool value);
private:
	void Initialise();

	std::map<std::string, bool> boolSettings;
};
