// Copyright (c) 2019, Martin Linklater
// All rights reserved.
//
// See file 'LICENSE' for license details

#pragma once

#include <string>

namespace StringUtils
{
/**
 * @brief Get the string which is contained between the other two. If not found
 * return empty string
 * 
 * @param str 
 * @param start 
 * @param end 
 * @return std::string 
 */
std::string GetContained(	const std::string& str,
							const std::string& start,
							const std::string& end);
}
