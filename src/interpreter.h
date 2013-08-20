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
 * interpreter.h
 *
 *  Created on: 2013-08-20
 *      Author: nicholas
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_
#include "parser.h"

namespace gcode
{

/*
Takes low level gcode and interprets it and provides higher level callbacks
e.g. linear, arc, etc.
intentionally limited in scope.
*/
class interpreter : public parser
{
private:
	virtual void begin_block(std::size_t line_no, bool block_delete)
	{
	
	}
	virtual void block_number(double block_no)
	{
	
	}
	virtual void word(char code, double value)
	{
	
	}
	virtual void comment(const char* begin, const char* end)
	{
	
	}
	virtual void end_block()
	{
	
	}
public:
	
};

}

#endif /* INTERPRETER_H_ */
