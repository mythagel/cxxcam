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

GCodeBlock& GCodeBlock::operator+=(const GCodeLine& line)
{
	m_Lines.push_back(line);
	return *this;
}

GCodeBlock& GCodeBlock::operator+=(const GCodeWord& word)
{
	if(m_Lines.empty())
		m_Lines.emplace_back(word);
	else
		m_Lines.back() += word;
	return *this;
}
void GCodeBlock::NewLine()
{
	m_Lines.emplace_back();
}

std::string GCodeBlock::str() const
{
	if(m_Lines.empty())
		return "";

	std::stringstream s;

	if(!m_Name.empty())
	{
		GCodeLine c;
		c.Comment(m_Name);
		s << c.str();
	}

	for(std::vector<GCodeLine>::const_iterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
	{
		s << it->str();
	}

	return s.str();
}
