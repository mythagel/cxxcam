/* cxxcam - C++ CAD/CAM driver library.
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
 * gcode.h
 *
 *  Created on: 2013-08-17
 *      Author: nicholas
 */

#ifndef POSTPGCODE_H_
#define POSTPGCODE_H_
#include <cstddef>
#include <cctype>
#include <stdexcept>

namespace postp
{

struct parser_t
{
	virtual void begin_block(std::size_t line_no);
	virtual void word(char code, double value);
	virtual void word(char code, int value);
	virtual void comment(const char* cmt);
	virtual void end_block();
	
	double parse_double(const char*& c, const char* const end)
	{
		const char* start = c;
		
		auto valid_first = [](char c) -> bool
		{
			if(isdigit(c))
				return true;
			if(c == '+' || c == '-')
				return true;
			if(c == '.')
				return true;
			return false;
		};

		if(!valid_first(*c))
			throw std::runtime_error("invalid character.");
		
		bool has_point = (*c == '.');
		bool has_digit = isdigit(*c);
		++c;
		
		while(c != end)
		{
			if(isdigit(*c))
			{
				has_digit = true;
				++c;
			}
			else if(*c == '.')
			{
				if(has_point)
					break;
				has_point = true;
				++c;
			}
			else
				break;
		}
		
		if(!has_digit)
			throw std::runtime_error("expected digits");
		
		return std::strtod(start, nullptr);
	}
	
	void parse(const char* c, const char* const end)
	{
		std::size_t line_no = 1;
		while(c != end)
		{
			switch(*c)
			{
				case '\r':
				{
					++c;
					if(c != end && *c == '\n')
						++c;
				
					++line_no;
					end_block();
					if(c != end)
						begin_block(line_no);
					continue;
				}
				case '\n':
				{
					++c;
					++line_no;
					end_block();
					if(c != end)
						begin_block(line_no);
					continue;
				}
				case 'A':
				case 'a':
				{
					word('A', parse_double(c, end));
					break;
				}
			}
			
			++c;
		}
	}
};

	// gcode callback based parser
	
}

#endif /* POSTPGCODE_H_ */
