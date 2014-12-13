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
 * GCode.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "GCode.h"
#include <sstream>
#include <algorithm>
#include <iterator>
#include "Error.h"

namespace cxxcam
{
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
	else if(variant == "Generic")
		m_Variant = variant_LinuxCNC;
	else
		throw error("Unknown GCode variant");

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

void Code::SetCallback(std::function<void(const std::vector<Word>&, const std::string&)> fn)
{
    m_Callback = fn;
}

auto Code::begin() const -> const_iterator
{
	return m_Blocks.begin();
}
auto Code::end() const -> const_iterator
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
	if(m_Callback) m_Callback({line.begin(), line.end()}, line.Comment());

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

const Block& Code::CurrentBlock()
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
}

