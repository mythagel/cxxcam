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
 * GCodeBlock.h
 *
 *  Created on: 28/04/2012
 *      Author: nicholas
 */

#ifndef GCODEBLOCK_H_
#define GCODEBLOCK_H_
#include <string>
#include <vector>
#include "GCodeLine.h"
#include "MachineState.h"

namespace cxxcam
{
namespace gcode
{

/*
 * A block is a collective sequence of Lines with a similar purpose.
 * Not to be confused with a GCode 'block' which is a synonym for line
 */
class Block
{
private:
	std::string m_Name;
	std::vector<Line> m_Lines;

	MachineState m_InitialState;
public:

	typedef std::vector<Line>::const_iterator const_iterator;

	Block(const std::string& name, const MachineState& initial_state);

	std::string Name() const;
	MachineState State() const;

	const_iterator begin() const;
	const_iterator end() const;
	bool empty() const;

	void append(const Line& line);
	void append(const Word& word);

	void NewLine();

	std::string debug_str() const;
};

}
}

#endif /* GCODEBLOCK_H_ */
