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
 * GCodeBlock.cpp
 *
 *  Created on: 28/04/2012
 *      Author: nicholas
 */

#include "GCodeBlock.h"
#include <sstream>

namespace cxxcam
{
namespace gcode
{

Block::Block(const std::string& name, const MachineState& initial_state)
 : m_Name(name), m_InitialState(initial_state)
{
}

std::string Block::Name() const
{
	return m_Name;
}
MachineState Block::State() const
{
	return m_InitialState;
}

auto Block::begin() const -> const_iterator
{
	return m_Lines.begin();
}
auto Block::end() const -> const_iterator
{
	return m_Lines.end();
}
bool Block::empty() const
{
	return m_Lines.empty();
}

void Block::append(const Line& line)
{
	m_Lines.push_back(line);
}

void Block::append(const Word& word)
{
	if(m_Lines.empty())
		m_Lines.emplace_back(word);
	else
		m_Lines.back() += word;
}
void Block::NewLine()
{
	m_Lines.emplace_back();
}

std::string Block::debug_str() const
{
	if(empty())
		return {};

	std::ostringstream s;

	if(!m_Name.empty())
		s << Line(m_Name).debug_str();

	for(auto& line : m_Lines)
		s << line.debug_str();

	return s.str();
}

}
}

