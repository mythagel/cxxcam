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

namespace gcode
{

class parser
{
public:
	virtual void begin_block(std::size_t line_no, bool block_delete);
	virtual void block_number(char code, std::size_t block_no);
	virtual void word(char code, double value);
	virtual void word(char code, int value);
	// TODO parameters
	virtual void comment(const char* text, std::size_t len);
	virtual void end_block();
	
	void parse(const char* c, const char* end)
	{
	
	}
};

}

#endif /* PARSER_H_ */
