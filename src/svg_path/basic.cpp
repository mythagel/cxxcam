/* 
 * Copyright (C) 2013  Nicholas Gill
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * basic.cpp
 *
 *  Created on: 2013-12-05
 *      Author: nicholas
 */

#include "basic.h"
#include <cctype>
#include <stdexcept>
#include <cstdlib>

namespace svg
{
namespace types
{
namespace parsers
{

namespace
{
inline bool throw_if(bool cond, const std::string& what)
{
    if(cond) throw std::invalid_argument(what);
    return cond;
}
}

bool ws_p(const char c)
{
	switch(c)
	{
		case '\t':
		case '\n':
		case '\r':
		case ' ':
			return true;
		default:
			return false;
	}
}

bool parse_whitespace(const char*& c, const char* const end)
{
    if(!ws_p(*c))
        return false;

    while(c != end && ws_p(*c))
        ++c;
    return true;
}

bool number_p(const char c)
{
	switch(c)
	{
		case '+':
		case '-':
		case '.':
			return true;
		default:
			return std::isdigit(c);
	}
}

bool parse_number(const char*& c, const char* const end, float& x)
{
    if(!number_p(*c))
        return false;

    const auto begin = c;

    errno = 0;
    x = strtof(c, const_cast<char**>(&c));
    throw_if(c == begin || errno, "expected number");
    throw_if(c > end, "unexpected eof; strtof consumed too much");
    return true;
}

bool parse_comma_wsp(const char*& c, const char* const end)
{
    if(!ws_p(*c) && *c != ',')
        return false;

    if(parse_whitespace(c, end) && c == end)
        return true;

    if(*c == ',' && ++c == end)
        return true;

    parse_whitespace(c, end);
    return true;
}

bool parse_bool(const std::string& str)
{
    if(str == "true")
        return true;
    else if(str == "false")
        return false;
    throw std::invalid_argument("invalid value for bool: " + str);
}

}
}
}

