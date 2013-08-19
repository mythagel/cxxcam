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
 * parser.cpp
 *
 *  Created on: 2013-08-19
 *      Author: nicholas
 */

#include "parser.h"
#include <cstddef>
#include <cctype>

namespace gcode
{

void parser::parse_comment(const char*& c, const char* end)
{
	if(*c != '(')
		throw std::logic_error("parse_comment called on non-comment.");
	
	const char* begin = ++c;
	while(c != end)
	{
		if(*c == '\r' || *c == '\n')
		{
			throw expected_character(")");
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

double parser::read_number(const char*& c, const char* end)
{
	auto valid_first = [](char c) -> bool
	{
		return c == '+' || c == '-' || c == '.' || std::isdigit(c);
	};
	
	const char* begin = c;
	if(!valid_first(*c))
		throw expected_character("0-9/+/-/.");
	
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
		throw expected_character("0-9+");
	
	char* str_end;
	auto x = std::strtod(begin, &str_end);
	if(str_end != c)
		throw std::logic_error("strtod consumed more digits than parsed.");
	return x;
}

void parser::parse_block_number(const char*& c, const char* end)
{
	if(*c != 'N' && *c != 'n')
		throw std::logic_error("parse_block_number called on non-block-number");
	
	block_number(read_number(c, end));
}

void parser::parse_word(const char*& c, const char* end)
{
	if(!isalpha(*c))
		throw expected_character("a-zA-Z");
	char code = *c++;
	word(code, read_number(c, end));
}

void parser::parse(const char*& c, const char* end)
{
	std::size_t line_no = 1;
	bool in_block = false;
	
	auto open_block = [&line_no, &in_block, this](bool del = false)
	{
		if(!in_block)
		{
			begin_block(line_no, del);
			in_block = true;
		}
	};
	auto close_block = [&in_block, this]
	{
		if(in_block)
			end_block();
		in_block = false;
	};
	
	while(c != end)
	{
		switch(*c)
		{
			case '\r':
			{
				++c;
				++line_no;
				close_block();
				
				if(c != end && *c == '\n')
					++c;
				break;
			}
			case '\n':
			{
				++c;
				++line_no;
				close_block();
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
				if(in_block)
					throw unexpected_character("/");
				
				open_block(true);
				++c;
				break;
			}
			case 'N':
			case 'n':
			{
				if(in_block)
					throw unexpected_character("Nn");
				
				open_block();
				parse_block_number(c, end);
				break;
			}
			case '(':
			{
				open_block();
				parse_comment(c, end);
				break;
			}
			default:
			{
				open_block();
				parse_word(c, end);
				break;
			}
		}
	}
	close_block();
}

parser::~parser()
{
}

}

