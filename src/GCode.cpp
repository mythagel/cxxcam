/*
 * GCode.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "GCode.h"
#include <sstream>
#include <algorithm>
#include <iterator>

namespace gcode
{

const char* Code::eol() const
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

Code::Code(const std::string& variant)
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

Code::const_iterator Code::begin() const
{
	return m_Blocks.begin();
}
Code::const_iterator Code::end() const
{
	return m_Blocks.end();
}
bool Code::empty() const
{
	return m_Blocks.empty();
}

bool Code::AddLine(const Line& line)
{
	if(m_Blocks.empty())
		m_Blocks.emplace_back("", MachineState());

	m_Blocks.back().append(line);
	return true;
}

void Code::NewBlock(const std::string& name, const MachineState& initial_state)
{
	/*
	 * If the last block on the stack is empty, rename and reuse it.
	 */
	if(!m_Blocks.empty() && m_Blocks.back().empty())
	{
		if(m_Blocks.back().Name().empty())
			m_Blocks.back() = Block(name, initial_state);
		return;
	}
	m_Blocks.emplace_back(name, initial_state);
}

Block& Code::CurrentBlock()
{
	if(m_Blocks.empty())
		m_Blocks.emplace_back("", MachineState());

	return m_Blocks.back();
}

void Code::EndBlock()
{
	m_Blocks.emplace_back("", MachineState());
}

std::string Code::debug_str() const
{
	std::ostringstream s;
	for(auto& block : m_Blocks)
		s << block.debug_str() << '\n';

	return s.str();
}

std::ostream& operator<<(std::ostream& os, const Code& gcode)
{
	auto eol = gcode.eol();
	std::size_t block_id = 0;
	for(auto& block : gcode)
	{
		if(!block.Name().empty())
			os << "; " << block.Name() << eol;

		std::size_t line_id = 0;
		for(auto& line : block)
		{
			if(gcode.m_LineNumbers)
				os << 'N' << block_id << '.' << line_id << ' ';

			std::copy(line.begin(), line.end(), std::ostream_iterator<Word>(os, " "));
			if(!line.Comment().empty())
				os << "; " << line.Comment();

			os << eol;
			++line_id;
		}

		os << eol;
		++block_id;
	}
	return os;
}

}
