// Copyright (c) 2019, Martin Linklater
// All rights reserved.
//
// See file 'LICENSE' for license details

// log levels... fatal, error, warning, info, verbose

#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <stdarg.h>

#define LOGVERBOSE(x) Log::Instance()->Verbose(x)
#define LOGVERBOSEF(x, ...) Log::Instance()->Verbosef(x, __VA_ARGS__)

#define LOGINFO(x) Log::Instance()->Info(x)
#define LOGINFOF(x, ...) Log::Instance()->Infof(x, __VA_ARGS__)

#define LOGWARNING(x) Log::Instance()->Warning(x)
#define LOGWARNINGF(x, ...) Log::Instance()->Warningf(x, __VA_ARGS__)

#define LOGERROR(x) Log::Instance()->Error(x)
#define LOGERRORF(x, ...) Log::Instance()->Errorf(x, __VA_ARGS__)

#define LOGFATAL(x) Log::Instance()->Fatal(x)
#define LOGFATALF(x, ...) Log::Instance()->Fatalf(x, __VA_ARGS__)

class Log
{
public:
	enum eLogType
	{
		kVerbose,
		kInfo,
		kWarning,
		kError,
		kFatal
	};
	
	struct LogLine
	{
		eLogType 	type;
		std::string category;
		std::string content;
	};
	
	Log();
	virtual ~Log();
	static Log* Instance();
	
	void Test();
	
	void Verbose(const std::string& line);
	void Verbosef(const char* fmt, ...);

	void Info(const std::string& line);
	void Infof(const char* fmt, ...);
	
	void Warning(const std::string& line);
	void Warningf(const char* fmt, ...);
	
	void Error(const std::string& line);
	void Errorf(const char* fmt, ...);

	void Fatal(const std::string& line);
	void Fatalf(const char* fmt, ...);

	int GetLineCount();
	const LogLine& GetLine(int number);
	
	void ResetCategories();
	const std::set<std::string>& GetCategories();

	bool GetCategoryEnabled(std::string category);
	void SetCategoryEnabled(std::string category, bool enabled);
	
private:
	
	void SplitCategory(std::string line, std::string& categoryOut, std::string& lineOut);
	void AddLine(LogLine line);
	
	void RecalculateDisplayedLines();
	
	std::vector<LogLine> allLogLines;
	std::vector<LogLine> filteredLogLines;
	std::set<std::string> categories;
	std::map<std::string, bool> categoryFilter;
};
