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
private:
	virtual void begin_block(std::size_t line_no, bool block_delete) =0;
	virtual void block_number(double block_no) =0;
	virtual void word(char code, double value) =0;
	virtual void comment(const char* begin, const char* end) =0;
	virtual void end_block() =0;

	void parse_comment(const char*& c, const char* end);
	double read_number(const char*& c, const char* end);
	void parse_block_number(const char*& c, const char* end);
	void parse_word(const char*& c, const char* end);
public:
	void parse(const char*& c, const char* end);
	
	virtual ~parser();
};

}

#endif /* PARSER_H_ */
