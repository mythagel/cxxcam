/* postp - gcode utilities
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
 * parser.h
 *
 *  Created on: 2013-08-19
 *      Author: nicholas
 */

#ifndef PARSER_H_
#define PARSER_H_
#include <cstddef>
#include <stdexcept>
#include <cctype>

namespace gcode
{

class parser
{
public:
	virtual void begin_block(std::size_t line_no, bool block_delete) =0;
	virtual void block_number(unsigned int block_no) =0;
	virtual void word(char code, double value) =0;
	virtual void comment(const char* begin, const char* end) =0;
	virtual void end_block() =0;
private:
	void parse_comment(const char*& c, const char* end)
	{
		if(*c != '(')
			throw std::logic_error("parse_comment: Unexpected character.");
		
		const char* begin = ++c;
		while(c != end)
		{
			if(*c == '\r' || *c == '\n')
			{
				throw std::runtime_error("parse_comment: Expected ')'");
			}
			else if(*c == ')')
			{
				++c;
				comment(begin, c);
				break;
			}
			++c;
		}
	}
	
	unsigned long read_ulong(const char*& c, const char* end)
	{
		auto valid_first = [](char c) -> bool
		{
			return c == '+' || c == '-' || c == '.' || std::isdigit(c);
		};
		
		const char* begin = c;
		if(!valid_first(*c))
			throw std::logic_error("read_ulong: Expected 0-9/+/-/.");
		bool has_point = *c == '.';
		bool has_digit = std::isdigit(*c);
		bool has_sign = *c == '-';
		++c;
		while(c != end)
		{
			if(isdigit(*c))
			{
				has_digit = true;
			}
			else if(*c == '.')
			{
				if(!has_point)
					has_point = true;
				else
					break;
			}
			else
			{
				break;
			}
			++c;
		}
		
		if(!has_digit)
			throw std::logic_error("read_ulong: Expected digits");
		if(has_point || has_sign)
			throw std::logic_error("read_ulong: Expected unsigned long");
		
		char* str_end;
		auto x = std::strtoul(begin, &str_end, 10);
		if(str_end != c)
			throw std::logic_error("read_ulong: strtoul consumed more digits than parsed.");
		return x;
	}
	double read_double(const char*& c, const char* end)
	{
		auto valid_first = [](char c) -> bool
		{
			return c == '+' || c == '-' || c == '.' || std::isdigit(c);
		};
		
		const char* begin = c;
		if(!valid_first(*c))
			throw std::logic_error("read_double: Expected 0-9/+/-/.");
		bool has_point = *c == '.';
		bool has_digit = std::isdigit(*c);
		++c;
		while(c != end)
		{
			if(isdigit(*c))
			{
				has_digit = true;
			}
			else if(*c == '.')
			{
				if(!has_point)
					has_point = true;
				else
					break;
			}
			else
			{
				break;
			}
			++c;
		}
		
		if(!has_digit)
			throw std::logic_error("read_double: Expected digits");
		
		char* str_end;
		auto x = std::strtod(begin, &str_end);
		if(str_end != c)
			throw std::logic_error("read_double: strtoul consumed more digits than parsed.");
		return x;
	}
	
	void parse_block_number(const char*& c, const char* end)
	{
		if(*c != 'N' && *c != 'n')
			throw std::logic_error("parse_block_number: Unexpected character.");
		
		block_number(read_ulong(c, end));
	}
	
	void parse_word(const char*& c, const char* end)
	{
		if(!isalpha(*c))
			throw std::runtime_error("parse_word: Expected alpha.");
		char code = *c;
		++c;
		word(code, read_double(c, end));
	}

public:
	void parse(const char*& c, const char* end)
	{
		std::size_t line_no = 1;
		bool in_block = false;
		
		while(c != end)
		{
			switch(*c)
			{
				case '\r':
				{
					++c;
					++line_no;
					
					if(in_block)
						end_block();
					in_block = false;
					
					if(c != end && *c == '\n')
						++c;
					break;
				}
				case '\n':
				{
					++c;
					++line_no;
				
					if(in_block)
						end_block();
					in_block = false;
					break;
				}
				case ' ':
				case '\t':
				{
					++c;
					break;
				}
				case '/':
				{
					if(!in_block)
					{
						begin_block(line_no, true);
						in_block = true;
						++c;
						break;
					}
					throw std::runtime_error("parse: Unexpected character.");
				}
				case 'N':
				case 'n':
				{
					if(!in_block)
					{
						begin_block(line_no, false);
						in_block = true;
					}
					parse_block_number(c, end);
					break;
				}
				case '(':
				{
					if(!in_block)
					{
						begin_block(line_no, false);
						in_block = true;
					}
					parse_comment(c, end);
					break;
				}
				default:
				{
					if(!in_block)
					{
						begin_block(line_no, false);
						in_block = true;
					}
					parse_word(c, end);
					break;
				}
			}
		}
	}
	
	virtual ~parser();
};

}

#endif /* PARSER_H_ */
