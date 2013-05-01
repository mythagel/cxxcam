/*
 * GCodeBlock.cpp
 *
 *  Created on: 28/04/2012
 *      Author: nicholas
 */

#include "GCodeBlock.h"
#include <sstream>

GCodeBlock::GCodeBlock(const std::string& name, const MachineState& initial_state)
 : m_Name(name), m_InitialState(initial_state)
{
}

std::string GCodeBlock::Name() const
{
	return m_Name;
}
MachineState GCodeBlock::State() const
{
	return m_InitialState;
}

GCodeBlock::const_iterator GCodeBlock::begin() const
{
	return m_Lines.begin();
}
GCodeBlock::const_iterator GCodeBlock::end() const
{
	return m_Lines.end();
}
bool GCodeBlock::empty() const
{
	return m_Lines.empty();
}

void GCodeBlock::append(const GCodeLine& line)
{
	m_Lines.push_back(line);
}

void GCodeBlock::append(const GCodeWord& word)
{
	if(m_Lines.empty())
		m_Lines.emplace_back(word);
	else
		m_Lines.back() += word;
}
void GCodeBlock::NewLine()
{
	m_Lines.emplace_back();
}

std::string GCodeBlock::debug_str() const
{
	if(empty())
		return {};

	std::ostringstream s;

	if(!m_Name.empty())
		s << GCodeLine(m_Name).debug_str();

	for(auto& line : m_Lines)
		s << line.debug_str();

	return s.str();
}
