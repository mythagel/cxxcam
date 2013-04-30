/*
 * GCode.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "GCode.h"
#include <iostream>
#include <sstream>

const char* GCode::eol() const
{
	switch(m_EndOfLine)
	{
		case CR:
			return "\r";
		case LF:
			return "\n";
		case CRLF:
			return "\r\n";
	}
	return "\n";
}

GCode::GCode(const std::string& variant)
{
	if(variant == "LinuxCNC")
		m_Variant = variant_LinuxCNC;
	else
		m_Variant = variant_LinuxCNC;

	switch(m_Variant)
	{
		case variant_LinuxCNC:
		{
			m_LineNumbers = false;
			m_Precision = 6;
			m_UpperCase = true;
			m_EndOfLine = LF;
			break;
		}
	}
}

bool GCode::AddLine(const GCodeLine& line)
{
	if(m_Blocks.empty())
		m_Blocks.emplace_back("", MachineState());

	m_Blocks.back() += line;
	return true;
}

void GCode::NewBlock(const std::string& name, const MachineState& initial_state)
{
	/*
	 * If the last block on the stack is empty, rename and reuse it.
	 */
	if(!m_Blocks.empty() && m_Blocks.back().empty())
	{
		if(m_Blocks.back().Name().empty())
			m_Blocks.back() = GCodeBlock(name, initial_state);
		return;
	}
	m_Blocks.emplace_back(name, initial_state);
}

GCodeBlock& GCode::CurrentBlock()
{
	if(m_Blocks.empty())
		m_Blocks.emplace_back("", MachineState());

	return m_Blocks.back();
}

void GCode::EndBlock()
{
	m_Blocks.emplace_back("", MachineState());
}

std::string GCode::str() const
{
	std::ostringstream s;

	for(auto& block : m_Blocks)
	{
		if(!block.empty())
			s << block.str() << eol();
	}

	return s.str();
}
