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
 * postp.cpp
 *
 *  Created on: 2013-08-19
 *      Author: nicholas
 */
#include "parser.h"
#include <iostream>
#include <vector>
#include <iterator>

class parser : public gcode::parser
{
public:
	virtual void begin_block(std::size_t line_no, bool block_delete) override
	{
		std::cout << "(@ " << line_no << ") " << (block_delete? "/" : "") << " ";
	}
	virtual void block_number(double block_no) override
	{
		std::cout << "N" << block_no << " ";
	}
	virtual void word(char code, double value) override
	{
		std::cout << code << value << " ";
	}
	virtual void comment(const char* begin, const char* end) override
	{
		std::cout << "(" << std::string(begin, end) << ") ";
	}
	virtual void end_block() override
	{
		std::cout << "\n";
	}
	
	virtual ~parser()
	{
	}
};

int main()
{
	/*
	Launch post processor utilities based on name
	e.g. postp rename-axis C A source.ngc
	Would search ~/postp/rename-axis.js && /usr/share/postp/rename-axis.js
	
	Need to consider configuration and interface
	e.g. postp model source.ngc
	How to specify machine configuration & stock etc.
	*/
	
	std::istream* is = &std::cin;

	std::noskipws(*is);
	std::vector<char> source{std::istream_iterator<char>(*is), std::istream_iterator<char>()};
	
	const char* begin = source.data();
	const char* end = source.data() + source.size();
	
	parser p;
	p.parse(begin, end);
	
	return 0;
}
