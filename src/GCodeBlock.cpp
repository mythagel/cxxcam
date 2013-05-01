/*
 * GCodeBlock.cpp
 *
 *  Created on: 28/04/2012
 *      Author: nicholas
 */

#include "GCodeBlock.h"
#include <sstream>

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

Block::const_iterator Block::begin() const
{
	return m_Lines.begin();
}
Block::const_iterator Block::end() const
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
